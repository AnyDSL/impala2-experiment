#ifndef IMPALA_AST_H
#define IMPALA_AST_H

#include <deque>
#include <memory>

#include "thorin/util/cast.h"
#include "thorin/util/stream.h"

#include "impala/token.h"
#include "impala/print.h"

namespace impala {

class Scopes;

struct Expr;
struct Stmnt;

class Printer;

template<class T> using Ptr = std::unique_ptr<T>;
template<class T> using Ptrs = std::deque<std::unique_ptr<T>>;
template<class T, class... Args>
std::unique_ptr<T> make_ptr(Args... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

namespace detail {
    template<class T> void make_ptrs(Ptrs<T>&) {}
    template<class T, class A, class... Args>
    void make_ptrs(Ptrs<T>& ptrs, A&& arg, Args&&... args) {
        ptrs.emplace_back(std::forward<A>(arg));
        make_ptrs(ptrs, std::forward<Args>(args)...);
    }
}

template<class T, class... Args>
Ptrs<T> make_ptrs(Args&&... args) {
    Ptrs<T> result;
    detail::make_ptrs(result, std::forward<Args>(args)...);
    return result;
}

//------------------------------------------------------------------------------

struct Node : public thorin::Streamable<Printer> {
    Node(Location location)
        : location(location)
    {}
    virtual ~Node() {}

    std::ostream& stream_out(std::ostream&) const;

    Location location;
};

struct Id : public Node {
    Id(Token token)
        : Node(token.location())
        , symbol(token.symbol())
    {}

    Printer& stream(Printer&) const override;

    Symbol symbol;
};

/*
 * Ptrn
 */

struct Ptrn : public thorin::RuntimeCast<Ptrn>, public Node {
    Ptrn(Location location, Ptr<Expr>&& type, bool type_mandatory)
        : Node(location)
        , type(std::move(type))
        , type_mandatory(type_mandatory)
    {}

    virtual void bind(Scopes&) const = 0;
    Printer& stream_ascription(Printer&) const ;

    Ptr<Expr> type;
    bool type_mandatory;
};

struct ErrorPtrn : public Ptrn {
    ErrorPtrn(Location location)
        : Ptrn(location, nullptr, false)
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;
};

struct IdPtrn : public Ptrn {
    IdPtrn(Location location, Ptr<Id>&& id, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(location, std::move(type), type_mandatory)
        , id(std::move(id))
    {}

    Symbol symbol() const { return id->symbol; }

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Id> id;
};

struct TuplePtrn : public Ptrn {
    TuplePtrn(Location location, Ptrs<Ptrn>&& elems, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(location, std::move(type), type_mandatory)
        , elems(std::move(elems))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptrs<Ptrn> elems;
};

/*
 * Expr
 */

struct Expr : public thorin::RuntimeCast<Expr>, public Node {
    Expr(Location location)
        : Node(location)
    {}

    virtual void bind(Scopes&) const = 0;
};

struct AppExpr : public Expr {
    AppExpr(Location location, Ptr<Expr>&& callee, Ptr<Expr>&& arg, bool cps)
        : Expr(location)
        , callee(std::move(callee))
        , arg(std::move(arg))
        , cps(cps)
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Expr> callee;
    Ptr<Expr> arg;
    bool cps;
};

struct BlockExpr : public Expr {
    BlockExpr(Location location, Ptrs<Stmnt>&& stmnts, Ptr<Expr>&& expr)
        : Expr(location)
        , stmnts(std::move(stmnts))
        , expr(std::move(expr))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptrs<Stmnt> stmnts;
    Ptr<Expr> expr;
};

struct BottomExpr : public Expr {
    BottomExpr(Location location)
        : Expr(location)
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;
};

struct ErrorExpr : public Expr {
    ErrorExpr(Location location)
        : Expr(location)
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;
};

struct IdExpr : public Expr {
    IdExpr(Ptr<Id>&& id)
        : Expr(id->location)
        , id(std::move(id))

    {}

    Symbol symbol() const { return id->symbol; }

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Id> id;
    mutable const IdPtrn* id_ptrn;
};

struct IfExpr : public Expr {
    IfExpr(Location location, Ptr<Expr>&& cond, Ptr<Expr>&& then_expr, Ptr<Expr>&& else_expr)
        : Expr(location)
        , cond(std::move(cond))
        , then_expr(std::move(then_expr))
        , else_expr(std::move(else_expr))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Expr> cond;
    Ptr<Expr> then_expr;
    Ptr<Expr> else_expr;
};

struct InfixExpr : public Expr {
    enum class Tag {
        O_tilde      = int(Token::Tag::O_tilde),
        O_and_and    = int(Token::Tag::O_and_and),
        O_or_or      = int(Token::Tag::O_or_or),
    };

    InfixExpr(Location location, Ptr<Expr>&& lhs, Tag tag, Ptr<Expr>&& rhs)
        : Expr(location)
        , lhs(std::move(lhs))
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Expr> lhs;
    Tag tag;
    Ptr<Expr> rhs;
};

struct FieldExpr : public Expr {
    FieldExpr(Location location, Ptr<Expr>&& lhs, Ptr<Id>&& id)
        : Expr(location)
        , lhs(std::move(lhs))
        , id(std::move(id))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Expr> lhs;
    Ptr<Id> id;
};

struct ForallExpr : public Expr {
    ForallExpr(Location location, Ptr<Ptrn>&& domain, Ptr<Expr>&& codomain)
        : Expr(location)
        , domain(std::move(domain))
        , codomain(std::move(codomain))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Ptrn> domain;
    Ptr<Expr> codomain;
};

struct ForExpr : public Expr {
    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;
};

struct LambdaExpr : public Expr {
    LambdaExpr(Location location, Ptr<Ptrn>&& domain, Ptr<Expr>&& codomain, Ptr<Expr>&& body)
        : Expr(location)
        , domain(std::move(domain))
        , codomain(std::move(codomain))
        , body(std::move(body))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Ptrn> domain;
    Ptr<Expr> codomain;
    Ptr<Expr> body;
};

struct MatchExpr : public Expr {
    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;
};

struct PackExpr : public Expr {
    PackExpr(Location location, Ptrs<Ptrn>&& domains, Ptr<Expr>&& body)
        : Expr(location)
        , domains(std::move(domains))
        , body(std::move(body))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptrs<Ptrn> domains;
    Ptr<Expr> body;
};

struct PrefixExpr : public Expr {
    enum class Tag {
        O_inc = int(Token::Tag::O_inc),
        O_dec = int(Token::Tag::O_dec),
        O_add = int(Token::Tag::O_add),
        O_sub = int(Token::Tag::O_sub),
        O_and = int(Token::Tag::O_and),
    };

    PrefixExpr(Location location, Tag tag, Ptr<Expr>&& rhs)
        : Expr(location)
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Tag tag;
    Ptr<Expr> rhs;
};

struct PostfixExpr : public Expr {
    enum class Tag {
        O_inc = int(Token::Tag::O_inc),
        O_dec = int(Token::Tag::O_dec),
    };

    PostfixExpr(Location location, Ptr<Expr>&& lhs, Tag tag)
        : Expr(location)
        , lhs(std::move(lhs))
        , tag(tag)
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Expr> lhs;
    Tag tag;
};

struct TupleExpr : public Expr {
    struct Elem : public Node {
        Elem(Location location, Ptr<Id>&& id, Ptr<Expr>&& expr)
            : Node(location)
            , id(std::move(id))
            , expr(std::move(expr))
        {}

        void bind(Scopes&) const;
        Printer& stream(Printer&) const override;

        Ptr<Id> id;
        Ptr<Expr> expr;
    };

    TupleExpr(Location location, Ptrs<Elem>&& elems, Ptr<Expr>&& type)
        : Expr(location)
        , elems(std::move(elems))
        , type(std::move(type))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptrs<Elem> elems;
    Ptr<Expr> type;
};

struct VariadicExpr : public Expr {
    VariadicExpr(Location location, Ptrs<Ptrn>&& domains, Ptr<Expr>&& body)
        : Expr(location)
        , domains(std::move(domains))
        , body(std::move(body))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptrs<Ptrn> domains;
    Ptr<Expr> body;
};

struct SigmaExpr : public Expr {
    SigmaExpr(Location location, Ptrs<Ptrn>&& elems)
        : Expr(location)
        , elems(std::move(elems))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptrs<Ptrn> elems;
};

struct UnknownExpr : public Expr {
    UnknownExpr(Location location)
        : Expr(location)
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;
};

struct WhileExpr : public Expr {
};

/*
 * Stmnt
 */

struct Stmnt : public thorin::RuntimeCast<Stmnt>, public Node {
    Stmnt(Location location)
        : Node(location)
    {}

    virtual void bind(Scopes&) const = 0;
};

struct ExprStmnt : public Stmnt {
    ExprStmnt(Location location, Ptr<Expr>&& expr)
        : Stmnt(location)
        , expr(std::move(expr))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Expr> expr;
};

struct LetStmnt : public Stmnt {
    LetStmnt(Location location, Ptr<Ptrn>&& ptrn, Ptr<Expr>&& init)
        : Stmnt(location)
        , ptrn(std::move(ptrn))
        , init(std::move(init))
    {}

    void bind(Scopes&) const override;
    Printer& stream(Printer&) const override;

    Ptr<Ptrn> ptrn;
    Ptr<Expr> init;
};

}

#endif

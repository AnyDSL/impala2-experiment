#ifndef IMPALA_AST_H
#define IMPALA_AST_H

#include <deque>
#include <memory>

#include "thorin/util/cast.h"
#include "thorin/util/stream.h"

#include "impala/token.h"
#include "impala/print.h"

namespace impala {

struct Expr;
struct Stmnt;

class Printer;

template<class T> using Ptr = std::unique_ptr<T>;
template<class T> using Ptrs = std::deque<std::unique_ptr<T>>;
template<class T, class... Args>
std::unique_ptr<T> make_ptr(Args... args) { return std::make_unique<T>(std::forward<Args>(args)...); }

struct Node : public thorin::RuntimeCast<Node>, public thorin::Streamable<Printer> {
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

struct Ptrn : public Node {
    Ptrn(Location location, Ptr<Expr>&& type, bool type_mandatory)
        : Node(location)
        , type(std::move(type))
        , type_mandatory(type_mandatory)
    {}

    Printer& stream_ascription(Printer&) const ;

    Ptr<Expr> type;
    bool type_mandatory;
};

struct ErrorPtrn : public Ptrn {
    ErrorPtrn(Location location)
        : Ptrn(location, nullptr, false)
    {}

    Printer& stream(Printer&) const override;
};

struct IdPtrn : public Ptrn {
    IdPtrn(Location location, Ptr<Id>&& id, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(location, std::move(type), type_mandatory)
        , id(std::move(id))
    {}

    Printer& stream(Printer&) const override;

    Ptr<Id> id;
};

struct TuplePtrn : public Ptrn {
    TuplePtrn(Location location, Ptrs<Ptrn>&& ptrns, Ptr<Expr>&& type, bool type_mandatory)
        : Ptrn(location, std::move(type), type_mandatory)
        , ptrns(std::move(ptrns))
    {}

    Printer& stream(Printer&) const override;

    Ptrs<Ptrn> ptrns;
};

/*
 * Expr
 */

struct Expr : public Node {
    Expr(Location location)
        : Node(location)
    {}
};

struct BlockExpr : public Expr {
    BlockExpr(Location location, Ptrs<Stmnt>&& stmnts, Ptr<Expr>&& expr)
        : Expr(location)
        , stmnts(std::move(stmnts))
        , expr(std::move(expr))
    {}

    Printer& stream(Printer&) const override;

    Ptrs<Stmnt> stmnts;
    Ptr<Expr> expr;
};

struct BottomExpr : public Expr {
    BottomExpr(Location location)
        : Expr(location)
    {}

    Printer& stream(Printer&) const override;
};

struct ErrorExpr : public Expr {
    ErrorExpr(Location location)
        : Expr(location)
    {}

    Printer& stream(Printer&) const override;
};

struct IdExpr : public Expr {
    IdExpr(Ptr<Id>&& id)
        : Expr(id->location)
        , id(std::move(id))

    {}

    Printer& stream(Printer&) const override;

    Ptr<Id> id;
};

struct IfExpr : public Expr {
    IfExpr(Location location, Ptr<Expr>&& cond, Ptr<Expr>&& then_expr, Ptr<Expr>&& else_expr)
        : Expr(location)
        , cond(std::move(cond))
        , then_expr(std::move(then_expr))
        , else_expr(std::move(else_expr))
    {}

    Printer& stream(Printer&) const override;

    Ptr<Expr> cond;
    Ptr<Expr> then_expr;
    Ptr<Expr> else_expr;
};

struct InfixExpr : public Expr {
    enum class Tag {
        O_eq         = int(Token::Tag::O_eq),
        O_add_eq     = int(Token::Tag::O_add_eq),
        O_sub_eq     = int(Token::Tag::O_sub_eq),
        O_mul_eq     = int(Token::Tag::O_mul_eq),
        O_div_eq     = int(Token::Tag::O_div_eq),
        O_rem_eq     = int(Token::Tag::O_rem_eq),
        O_shl_eq     = int(Token::Tag::O_shl_eq),
        O_shr_eq     = int(Token::Tag::O_shr_eq),
        O_and_eq     = int(Token::Tag::O_and_eq),
        O_or_eq      = int(Token::Tag::O_or_eq),
        O_xor_eq     = int(Token::Tag::O_xor_eq),
        O_add        = int(Token::Tag::O_add),
        O_sub        = int(Token::Tag::O_sub),
        O_mul        = int(Token::Tag::O_mul),
        O_div        = int(Token::Tag::O_div),
        O_rem        = int(Token::Tag::O_rem),
        O_tilde      = int(Token::Tag::O_tilde),
        O_shl        = int(Token::Tag::O_shl),
        O_shr        = int(Token::Tag::O_shr),
        O_and        = int(Token::Tag::O_and),
        O_and_and    = int(Token::Tag::O_and_and),
        O_or         = int(Token::Tag::O_or),
        O_or_or      = int(Token::Tag::O_or_or),
        O_xor        = int(Token::Tag::O_xor),
        O_not        = int(Token::Tag::O_not),
        O_cmp_le     = int(Token::Tag::O_cmp_le),
        O_cmp_ge     = int(Token::Tag::O_cmp_ge),
        O_cmp_lt     = int(Token::Tag::O_cmp_lt),
        O_cmp_gt     = int(Token::Tag::O_cmp_gt),
        O_cmp_eq     = int(Token::Tag::O_cmp_eq),
        O_cmp_ne     = int(Token::Tag::O_cmp_ne),
    };

    InfixExpr(Location location, Ptr<Expr>&& lhs, Tag tag, Ptr<Expr>&& rhs)
        : Expr(location)
        , lhs(std::move(lhs))
        , tag(tag)
        , rhs(std::move(rhs))
    {}

    Printer& stream(Printer&) const override;

    Ptr<Expr> lhs;
    Tag tag;
    Ptr<Expr> rhs;
};

struct ForallExpr : public Expr {
    ForallExpr(Location location, Ptr<Ptrn>&& domain, Ptr<Expr>&& codomain)
        : Expr(location)
        , domain(std::move(domain))
        , codomain(std::move(codomain))
    {}

    Printer& stream(Printer&) const override;

    Ptr<Ptrn> domain;
    Ptr<Expr> codomain;
};


struct ForExpr : public Expr {
    Printer& stream(Printer&) const override;
};

struct LambdaExpr : public Expr {
    LambdaExpr(Location location, Ptr<Ptrn>&& domain, Ptr<Expr>&& codomain, Ptr<Expr>&& body)
        : Expr(location)
        , domain(std::move(domain))
        , codomain(std::move(codomain))
        , body(std::move(body))
    {}

    Printer& stream(Printer&) const override;

    Ptr<Ptrn> domain;
    Ptr<Expr> codomain;
    Ptr<Expr> body;
};

struct MatchExpr : public Expr {
    Printer& stream(Printer&) const override;
};

struct PackExpr : public Expr {
    PackExpr(Location location, Ptrs<Ptrn>&& domains, Ptr<Expr>&& body)
        : Expr(location)
        , domains(std::move(domains))
        , body(std::move(body))
    {}

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

        Printer& stream(Printer&) const override;

        Ptr<Id> id;
        Ptr<Expr> expr;
    };

    TupleExpr(Location location, Ptrs<Elem>&& elems, Ptr<Expr>&& type)
        : Expr(location)
        , elems(std::move(elems))
        , type(std::move(type))
    {}

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

    Printer& stream(Printer&) const override;

    Ptrs<Ptrn> domains;
    Ptr<Expr> body;
};

struct SigmaExpr : public Expr {
    SigmaExpr(Location location, Ptrs<Ptrn>&& elems)
        : Expr(location)
        , elems(std::move(elems))
    {}

    Printer& stream(Printer&) const override;

    Ptrs<Ptrn> elems;
};

struct UnknownExpr : public Expr {
    UnknownExpr(Location location)
        : Expr(location)
    {}

    Printer& stream(Printer&) const override;
};

struct WhileExpr : public Expr {
};

/*
 * Stmnt
 */

struct Stmnt : public Node {
    Stmnt(Location location)
        : Node(location)
    {}
};

struct ExprStmnt : public Stmnt {
    ExprStmnt(Location location, Ptr<Expr>&& expr)
        : Stmnt(location)
        , expr(std::move(expr))
    {}

    Printer& stream(Printer&) const override;

    Ptr<Expr> expr;
};

struct LetStmnt : public Stmnt {
    LetStmnt(Location location, Ptr<Ptrn>&& ptrn, Ptr<Expr>&& init)
        : Stmnt(location)
        , ptrn(std::move(ptrn))
        , init(std::move(init))
    {}

    Printer& stream(Printer&) const override;

    Ptr<Ptrn> ptrn;
    Ptr<Expr> init;
};

}

#endif

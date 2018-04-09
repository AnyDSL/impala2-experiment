#ifndef IMPALA_AST_H
#define IMPALA_AST_H

#include <deque>
#include <memory>

#include "impala/token.h"

namespace impala {

struct Stmnt;

template<class T> using Ptr = std::unique_ptr<T>;
template<class T> using PtrDeque = std::deque<std::unique_ptr<T>>;

struct Node {
    Node(Location location)
        : location(location)
    {}
    virtual ~Node() {}

    Location location;
};

struct Id : public Node {
    Id(Location location, Symbol symbol)
        : Node(location)
        , symbol(symbol)
    {}
    Id(Token token)
        : Node(token.location())
        , symbol(token.symbol())
    {}

    //std::ostream& stream(std::ostream&) const override;

    Symbol symbol;
};

/*
 * Ptrn
 */

struct Ptrn : public Node {
    Ptrn(Location location)
        : Node(location)
    {}
};

struct IdPtrn : public Ptrn {
    IdPtrn(Location location, Ptr<Id>&& id)
        : Ptrn(location)
        , id(std::move(id))

    {}

    Ptr<Id> id;
};

struct TuplePtrn : public Ptrn {
    TuplePtrn(Location location, PtrDeque<Ptrn>&& ptrns)
        : Ptrn(location)
        , ptrns(std::move(ptrns))
    {}

    PtrDeque<Ptrn> ptrns;
};

/*
 * Expr
 */

struct Expr : public Node {
    Expr(Location location)
        : Node(location)
    {}
};

struct BinderExpr : public Expr {
    BinderExpr(Location location, Ptr<Id>&& id, Ptr<Expr> type)
        : Expr(location)
        , id(std::move(id))
        , type(std::move(type))
    {}

    Ptr<Id> id;
    Ptr<Expr> type;
};

struct BlockExpr : public Expr {
    BlockExpr(Location location, PtrDeque<Stmnt>&& stmnts, Ptr<Expr>&& expr)
        : Expr(location)
        , stmnts(std::move(stmnts))
        , expr(std::move(expr))
    {}

    PtrDeque<Stmnt> stmnts;
    Ptr<Expr> expr;
};

struct TupleExpr : public Expr {
    TupleExpr(Location location, PtrDeque<Expr>&& exprs)
        : Expr(location)
        , exprs(std::move(exprs))
    {}
    TupleExpr(Location location)
        : TupleExpr(location, {})
    {}

    PtrDeque<Expr> exprs;
};

struct SigmaExpr : public Expr {
    SigmaExpr(Location location, PtrDeque<BinderExpr>&& binders)
        : Expr(location)
        , binders(std::move(binders))
    {}
    SigmaExpr(Location location)
        : SigmaExpr(location, {})
    {}

    PtrDeque<BinderExpr> binders;
};

struct IfExpr : public Expr {
    IfExpr(Location location, Ptr<Expr>&& cond, Ptr<Expr>&& then_expr, Ptr<Expr>&& else_expr)
        : Expr(location)
        , cond(std::move(cond))
        , then_expr(std::move(then_expr))
        , else_expr(std::move(else_expr))
    {}

    Ptr<Expr> cond;
    Ptr<Expr> then_expr;
    Ptr<Expr> else_expr;
};

/*
 * Stmnt
 */

struct Stmnt : public Node {
    Stmnt(Location location)
        : Node(location)
    {}
};

struct ExprStmt : public Stmnt {
    ExprStmt(Location location, Ptr<Expr>&& expr)
        : Stmnt(location)
        , expr(std::move(expr))
    {}

    Ptr<Expr> expr;
};

}

#endif

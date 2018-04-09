#ifndef IMPALA_AST_H
#define IMPALA_AST_H

#include <deque>
#include <memory>

#include "impala/token.h"

namespace impala {

template<class T> using Ptr = std::unique_ptr<T>;
template<class T> using PtrDeque = std::deque<std::unique_ptr<T>>;

struct Node {
    Node(Location location)
        : location(location)
    {}

    Location location;
};

struct Stmnt : public Node {
    Stmnt(Location location)
        : Node(location)
    {}
};

struct Expr : public Node {
    Expr(Location location)
        : Node(location)
    {}
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
    TupleExpr(Location location, PtrDeque<Expr>&& args)
        : Expr(location)
        , args(std::move(args))
    {}
    TupleExpr(Location location)
        : TupleExpr(location, {})
    {}

    PtrDeque<Expr> args;
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

}

#endif

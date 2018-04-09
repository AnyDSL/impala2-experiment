#ifndef IMPALA_AST_H
#define IMPALA_AST_H

#include <deque>
#include <memory>

namespace impala {

template<class T> using Ptr = std::unique_ptr<T>;
template <typename T> using PtrDeque = std::deque<std::unique_ptr<T>>;

struct Node {
};

struct Expr : public Node {
};

struct TupleExpr : public Expr {
};

struct IfExpr : public Expr {
};


}

#endif

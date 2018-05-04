#ifndef IMPALA_BIND_H
#define IMPALA_BIND_H

#include <variant>

#include "impala/compiler.h"

namespace impala {

using thorin::Symbol;

struct Node;
struct IdPtrn;
struct Item;
using Decl = std::variant<const IdPtrn*, const Item*>;


/// Binds identifiers to the nodes of the AST.
class Scopes {
public:
    Scopes(Compiler& compiler)
        : compiler_(compiler)
    {}

    Compiler& compiler() { return compiler_; }

    /// Performs name binding on a whole program.
    void run(const Node*);

    //void bind_head(const ast::IdPtrn&);
    void bind(const Node*);

    void push() { scopes_.emplace_back(); }
    void pop()  { scopes_.pop_back(); }

    void insert(Decl);

    Decl find(Symbol symbol);

private:
    Compiler& compiler_;
    std::vector<thorin::SymbolMap<Decl>> scopes_;
};

}

#endif

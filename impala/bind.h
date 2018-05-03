#ifndef IMPALA_BIND_H
#define IMPALA_BIND_H

#include "impala/compiler.h"

namespace impala {

using thorin::Symbol;

struct Decl;
struct Node;

/// Binds identifiers to the nodes of the AST.
class Scopes {
public:
    Scopes(Compiler& compiler)
        : compiler_(compiler)
    {}

    /// Performs name binding on a whole program.
    void run(const Node*);

    //void bind_head(const ast::Decl&);
    void bind(const Node*);

    void push() { scopes_.emplace_back(); }
    void pop()  { scopes_.pop_back(); }
    //void insert_symbol(const ast::NamedDecl&);

    using Entry = std::pair<Symbol, const Decl*>;

    Entry* find(Symbol symbol);

private:
    Compiler& compiler_;
    std::vector<thorin::SymbolMap<const Decl*>> scopes_;
};

}

#endif

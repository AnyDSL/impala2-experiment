#ifndef IMPALA_BIND_H
#define IMPALA_BIND_H

#include <variant>

#include "impala/compiler.h"

namespace impala {

using thorin::Symbol;

struct Id;
struct IdPtrn;
struct Item;
struct Node;

//------------------------------------------------------------------------------

// TODO use std::variant - but currently clang has a bug...
struct Decl {
    enum class Tag {
        None, IdPtrn, Item
    };

    Decl()
        : tag_(Tag::None)
    {}
    Decl(const IdPtrn* id_ptrn)
        : tag_(Tag::IdPtrn)
        , id_ptrn_(id_ptrn)
    {}
    Decl(const Item* item)
        : tag_(Tag::Item)
        , item_(item)
    {}

    bool is_valid() const { return tag_ != Tag::None; }
    const IdPtrn* id_ptrn() const { assert(tag_ == Tag::IdPtrn); return id_ptrn_; }
    const Item* item() const { assert(tag_ == Tag::Item); return item_; }
    const Id* id() const;
    Symbol symbol() const;

private:
    Tag tag_;
    union {
        const IdPtrn* id_ptrn_;
        const Item* item_;
    };
};

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

}

#endif

#ifndef IMPALA_BIND_H
#define IMPALA_BIND_H

#include <variant>

#include "impala/compiler.h"

namespace impala {

using thorin::Symbol;

template<class T> using Ptr = std::unique_ptr<const T>;
template<class T> using Ptrs = std::deque<Ptr<T>>;

struct Id;
struct IdPtrn;
struct Item;
struct Node;
struct Stmnt;

//------------------------------------------------------------------------------

struct Decl {
    enum class Tag { None, IdPtrn, Item };

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

    Tag tag() const { return tag_; }
    bool is_valid() const { return tag_ != Tag::None; }
    const IdPtrn* id_ptrn() const { assert(tag_ == Tag::IdPtrn); return id_ptrn_; }
    const Item* item() const { assert(tag_ == Tag::Item); return item_; }
    const Id* id() const;
    Symbol symbol() const;
    const thorin::Def* def() const;

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
    void bind(const Node*);
    void push() { scopes_.emplace_back(); }
    void pop()  { scopes_.pop_back(); }
    void insert(Decl);
    Decl find(Symbol symbol);
    void bind_stmnts(const Ptrs<Stmnt>&);

private:

    Compiler& compiler_;
    std::vector<thorin::SymbolMap<Decl>> scopes_;
};

//------------------------------------------------------------------------------

}

#endif

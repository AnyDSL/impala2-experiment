#include "impala/bind.h"

#include "impala/ast.h"

namespace impala {

//------------------------------------------------------------------------------

const Id* Decl::id() const {
    switch (tag_) {
        case Tag::IdPtrn: return id_ptrn_->id.get();
        case Tag::Item:   return item_->id.get();
        default: THORIN_UNREACHABLE;
    }
}

const thorin::Def* Decl::def() const {
    switch (tag_) {
        case Tag::IdPtrn: return id_ptrn_->def();
        case Tag::Item:   return item_->def();
        default: THORIN_UNREACHABLE;
    }
}

Symbol Decl::symbol() const { return id()->symbol; }

//------------------------------------------------------------------------------

Decl Scopes::find(Symbol symbol) {
    for (auto i = scopes_.rbegin(); i != scopes_.rend(); ++i) {
        auto& scope = *i;
        if (auto i = scope.find(symbol); i != scope.end())
            return i->second;
    }
    return Decl();
}

void Scopes::insert(Decl decl) {
    assert(!scopes_.empty());

    auto symbol = decl.symbol();
    if (symbol.is_anonymous()) return;

    if (auto [i, succ] = scopes_.back().emplace(symbol, decl); !succ) {
        compiler().error(decl.id()->loc, "redefinition of '{}'", symbol);
        compiler().note(i->second.id()->loc, "previous declaration of '{}' was here", symbol);
    }
}

void Scopes::bind_stmnts(const Ptrs<Stmnt>& stmnts) {
    auto i = stmnts.begin(), e = stmnts.end();
    while (i != e) {
        if ((*i)->isa<ItemStmnt>()) {
            for (auto j = i; j != e && (*j)->isa<ItemStmnt>(); ++j)
                (*j)->as<ItemStmnt>()->item->bind_rec(*this);
            for (; i != e && (*i)->isa<ItemStmnt>(); ++i)
                (*i)->as<ItemStmnt>()->item->bind(*this);
        } else {
            (*i)->bind(*this);
            ++i;
        }
    }
}

//------------------------------------------------------------------------------

void Prg::bind(Scopes& s) const {
    s.push();
    s.bind_stmnts(stmnts);
    s.pop();
}

void Item::bind(Scopes& s) const {
    s.push();
    expr->bind(s);
    s.pop();
}

void Item::bind_rec(Scopes& s) const {
    s.insert({this});
}

/*
 * Ptrn
 */

void IdPtrn::bind(Scopes& s) const {
    s.insert(this);
    type->bind(s);
}

void TuplePtrn::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
    type->bind(s);
}

void ErrorPtrn::bind(Scopes&) const {}

/*
 * Expr
 */

void AppExpr::bind(Scopes& s) const {
    callee->bind(s);
    arg->bind(s);
}

void BlockExpr::bind(Scopes& s) const {
    s.push();
    s.bind_stmnts(stmnts);
    expr->bind(s);
    s.pop();
}

void BottomExpr::bind(Scopes&) const {}

void FieldExpr::bind(Scopes& s) const {
    lhs->bind(s);
}

void ForallExpr::bind(Scopes& s) const {
    domain->bind(s);
    codomain->bind(s);
}

void IdExpr::bind(Scopes& s) const {
    if (!symbol().is_anonymous()) {
        decl = s.find(symbol());
        if (!decl.is_valid())
            s.compiler().error(loc, "use of undeclared identifier '{}'", symbol());
    } else {
        s.compiler().error(loc, "identifier '_' is reserved for anonymous declarations");
    }
}

void IfExpr::bind(Scopes& s) const {
    cond->bind(s);
    then_expr->bind(s);
    else_expr->bind(s);
}

void InfixExpr::bind(Scopes& s) const {
    lhs->bind(s);
    rhs->bind(s);
}

void LambdaExpr::bind(Scopes& s) const {
    domain->bind(s);
    codomain->bind(s);
    body->bind(s);
}

void PrefixExpr::bind(Scopes& s) const {
    rhs->bind(s);
}

void PostfixExpr::bind(Scopes& s) const {
    lhs->bind(s);
}

void QualifierExpr::bind(Scopes&) const {}

void TupleExpr::Elem::bind(Scopes& s) const {
    expr->bind(s);
}

void TupleExpr::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
    type->bind(s);
}

void UnknownExpr::bind(Scopes&) const {}

void PackExpr::bind(Scopes& s) const {
    for (auto&& domain : domains)
        domain->bind(s);
    body->bind(s);
}

void SigmaExpr::bind(Scopes& s) const {
    for (auto&& elem : elems)
        elem->bind(s);
}

void TypeExpr::bind(Scopes& s) const {
    qualifier->bind(s);
}

void VariadicExpr::bind(Scopes& s) const {
    for (auto&& domain : domains)
        domain->bind(s);
    body->bind(s);
}

void ErrorExpr::bind(Scopes&) const {}

/*
 * Stmnt
 */

void ExprStmnt::bind(Scopes& s) const {
    expr->bind(s);
}

void LetStmnt::bind(Scopes& s) const {
    if (init)
        init->bind(s);
    ptrn->bind(s);
}

void ItemStmnt::bind(Scopes& s) const {
    item->bind(s);
}

//------------------------------------------------------------------------------

}

#include "impala/emit.h"

#include "impala/ast.h"

namespace impala {

//------------------------------------------------------------------------------

void Emitter::emit_stmnts(const Ptrs<Stmnt>& stmnts) {
    auto i = stmnts.begin(), e = stmnts.end();
    while (i != e) {
        if ((*i)->isa<ItemStmnt>()) {
            for (auto j = i; j != e && (*j)->isa<ItemStmnt>(); ++j)
                (*j)->as<ItemStmnt>()->item->emit_rec(*this);
            for (; i != e && (*i)->isa<ItemStmnt>(); ++i)
                (*i)->as<ItemStmnt>()->item->emit(*this);
        } else {
            (*i)->emit(*this);
            ++i;
        }
    }
}

//------------------------------------------------------------------------------

void Prg::emit(Emitter& e) const {
    e.emit_stmnts(stmnts);
}

void Item::emit_rec(Emitter&) const {
    if (expr->isa<LambdaExpr>()) {
        def_ = nullptr; // TODO
    } else if (expr->isa<SigmaExpr>()) {
        def_ = nullptr; // TODO
    }
}

void Item::emit(Emitter& e) const {
    expr->emit(e);
}

/*
 * Ptrn
 */

const thorin::Def* Ptrn::emit(Emitter& e) const {
    auto t = type->emit(e);
    emit(e, t);
    return t;
}

void IdPtrn::emit(Emitter&, const thorin::Def* def) const {
    def_ = def;
}

void TuplePtrn::emit(Emitter& e, const thorin::Def* def) const {
    size_t i = 0;
    for (auto&& elem : elems)
        elem->emit(e, e.extract(def, i++, elems.size(), elem->loc));
}

void ErrorPtrn::emit(Emitter&, const thorin::Def*) const {}

/*
 * Expr
 */

const thorin::Def* AppExpr::emit(Emitter& e) const {
    auto c = callee->emit(e);
    auto a = arg->emit(e);
    return e.app(c, a, loc);
}

const thorin::Def* BlockExpr::emit(Emitter& e) const {
    e.emit_stmnts(stmnts);
    return expr->emit(e);
}

const thorin::Def* BottomExpr::emit(Emitter&) const {
    return nullptr;
}

const thorin::Def* FieldExpr::emit(Emitter& e) const {
    lhs->emit(e);
    return nullptr;
}

const thorin::Def* ForallExpr::emit(Emitter& e) const {
    auto d = domain->emit(e);
    auto c = codomain->emit(e);
    return e.pi(d, c, loc);
}

const thorin::Def* IdExpr::emit(Emitter&) const {
    switch (decl.tag()) {
        case Decl::Tag::IdPtrn:
            return decl.id_ptrn()->def();
        case Decl::Tag::Item: {
            auto item = decl.item();
            assert(item->def());
            return item->def();
        }
        case Decl::Tag::None:
            THORIN_UNREACHABLE;
    }
    THORIN_UNREACHABLE;
}

const thorin::Def* IfExpr::emit(Emitter& e) const {
    cond->emit(e);
    then_expr->emit(e);
    else_expr->emit(e);
    return nullptr;
}

const thorin::Def* InfixExpr::emit(Emitter& e) const {
    lhs->emit(e);
    rhs->emit(e);
    return nullptr;
}

const thorin::Def* LambdaExpr::emit(Emitter& e) const {
    auto d = domain->emit(e);
    //auto c = codomain->emit(e);
    auto b = body->emit(e);
    return e.lambda(d, b, loc);
}

const thorin::Def* PrefixExpr::emit(Emitter& e) const {
    rhs->emit(e);
    return nullptr;
}

const thorin::Def* PostfixExpr::emit(Emitter& e) const {
    lhs->emit(e);
    return nullptr;
}

const thorin::Def* QualifierExpr::emit(Emitter& e) const {
    switch (tag) {
        case Tag::u: return e.lit(thorin::Qualifier::u);
        case Tag::r: return e.lit(thorin::Qualifier::r);
        case Tag::a: return e.lit(thorin::Qualifier::a);
        case Tag::l: return e.lit(thorin::Qualifier::l);
        default: THORIN_UNREACHABLE;
    }
}

const thorin::Def* TupleExpr::Elem::emit(Emitter& e) const {
    return expr->emit(e);
}

const thorin::Def* TupleExpr::emit(Emitter& e) const {
    thorin::DefArray args(elems.size(), [&](size_t i) { return elems[i]->emit(e); });
    //type->emit(e);
    return e.tuple(args, loc);
}

const thorin::Def* UnknownExpr::emit(Emitter& e) const {
    return e.unknown(loc);
}

const thorin::Def* PackExpr::emit(Emitter& e) const {
    //for (auto&& domain : domains)
        //domain->emit(e);
    body->emit(e);
    return nullptr;
}

const thorin::Def* SigmaExpr::emit(Emitter& e) const {
    thorin::DefArray args(elems.size(), [&](size_t i) { return elems[i]->emit(e); });
    return e.sigma(args, loc);
}

const thorin::Def* TypeExpr::emit(Emitter& e) const {
    return e.kind_star(qualifier->emit(e));
}

const thorin::Def* VariadicExpr::emit(Emitter& e) const {
    //for (auto&& domain : domains)
        //domain->emit(e);
    body->emit(e);
    return nullptr;
}

const thorin::Def* ErrorExpr::emit(Emitter& e) const {
    return e.bot(e.kind_star());
}

/*
 * Stmnt
 */

void ExprStmnt::emit(Emitter& e) const {
    expr->emit(e);
}

void LetStmnt::emit(Emitter& e) const {
    auto i = init ? init->emit(e) : e.bot(e.kind_star());
    ptrn->emit(e, i);
}

void ItemStmnt::emit(Emitter&) const {
    //item->emit(e);
}

//------------------------------------------------------------------------------

}

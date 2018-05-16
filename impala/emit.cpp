#include "impala/emit.h"

#include "impala/ast.h"

namespace impala {

const thorin::Def* Item::emit(Emitter&) const {
    return nullptr;
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
    //emit_stmnts(stmnts, e);
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

const thorin::Def* IdExpr::emit(Emitter& e) const {
    switch (decl.tag()) {
        case Decl::Tag::IdPtrn: return decl.id_ptrn()->def();
        case Decl::Tag::Item: {
            auto item = decl.item();
            return item->def() ? item->def() : item->emit(e);
        }
        case Decl::Tag::None: THORIN_UNREACHABLE;
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
        case Tag::u: return e.qualifier(thorin::QualifierTag::u);
        case Tag::r: return e.qualifier(thorin::QualifierTag::r);
        case Tag::a: return e.qualifier(thorin::QualifierTag::a);
        case Tag::l: return e.qualifier(thorin::QualifierTag::l);
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

const thorin::Def* UnknownExpr::emit(Emitter&) const {
    return nullptr;
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
    return e.star(qualifier->emit(e));
}

const thorin::Def* VariadicExpr::emit(Emitter& e) const {
    //for (auto&& domain : domains)
        //domain->emit(e);
    body->emit(e);
    return nullptr;
}

const thorin::Def* ErrorExpr::emit(Emitter& e) const {
    return e.bottom(e.star());
}

/*
 * Stmnt
 */

void ExprStmnt::emit(Emitter& e) const {
    expr->emit(e);
}

void LetStmnt::emit(Emitter& e) const {
    auto i = init ? init->emit(e) : e.bottom(e.star());
    ptrn->emit(e, i);
}

void ItemStmnt::emit(Emitter&) const {
    //item->emit(e);
}

}

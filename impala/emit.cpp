#include "impala/emit.h"

#include "impala/ast.h"

namespace impala {

/*
 * Ptrn
 */

void IdPtrn::emit(Emitter&, const thorin::Def*) const {
}

void TuplePtrn::emit(Emitter&, const thorin::Def*) const {
}

void ErrorPtrn::emit(Emitter&, const thorin::Def*) const {
}

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
    //domain->emit(e);
    codomain->emit(e);
    return nullptr;
}

const thorin::Def* IdExpr::emit(Emitter&) const {
    return nullptr;
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
    //domain->emit(e);
    codomain->emit(e);
    body->emit(e);
    return nullptr;
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
    expr->emit(e);
    return nullptr;
}

const thorin::Def* TupleExpr::emit(Emitter& e) const {
    for (auto&& elem : elems)
        elem->emit(e);
    type->emit(e);
    return nullptr;
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
    auto sigma = e.sigma_type(elems.size());
    //for (auto&& elem : elems)
        //elem->emit(e);
    return sigma;
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
    if (init)
        init->emit(e);
    //ptrn->emit(e);
}

void ItemStmnt::emit(Emitter&) const {
    //item->emit(e);
}

}

#include "impala/bind.h"

#include "impala/ast.h"

namespace impala {

Decl Scopes::find(Symbol symbol) {
    for (auto i = scopes_.rbegin(); i != scopes_.rend(); ++i) {
        auto& scope = *i;
        if (auto i = scope.find(symbol); i != scope.end())
            return i->second;
    }
    return Decl((IdPtrn*)nullptr);
}

void Scopes::insert(Decl decl) {
    assert(!scopes_.empty());

    auto symbol = get_symbol(decl);
    if (symbol.is_anonymous()) return;

    if (auto [i, succ] = scopes_.back().emplace(symbol, decl); !succ) {
        compiler().error(get_id(decl)->loc, "redefinition of '{}'", symbol);
        compiler().note(get_id(i->second)->loc, "previous declaration of '{}' was here", symbol);
    }
}

//------------------------------------------------------------------------------

/*
 * Ptrn
 */

void IdPtrn::bind(Scopes& scopes) const {
    scopes.insert(this);
    type->bind(scopes);
}

void TuplePtrn::bind(Scopes& scopes) const {
    for (auto&& elem : elems)
        elem->bind(scopes);
    type->bind(scopes);
}

void ErrorPtrn::bind(Scopes&) const {}

/*
 * Expr
 */

void AppExpr::bind(Scopes& scopes) const {
    callee->bind(scopes);
    arg->bind(scopes);
}

void BlockExpr::bind(Scopes& scopes) const {
    scopes.push();
    for (auto&& stmnt : stmnts)
        stmnt->bind(scopes);
    expr->bind(scopes);
    scopes.pop();
}

void BottomExpr::bind(Scopes&) const {}

void FieldExpr::bind(Scopes& scopes) const {
    lhs->bind(scopes);
}

void ForallExpr::bind(Scopes& scopes) const {
    domain->bind(scopes);
    codomain->bind(scopes);
}

void IdExpr::bind(Scopes& scopes) const {
    if (!symbol().is_anonymous()) {
        decl = scopes.find(symbol());
        if (!is_valid(decl))
            scopes.compiler().error(loc, "use of undeclared identifier '{}'", symbol());
    } else {
        scopes.compiler().error(loc, "identifier '_' is reserved for anonymous declarations");
    }
}

void IfExpr::bind(Scopes& scopes) const {
    cond->bind(scopes);
    then_expr->bind(scopes);
    else_expr->bind(scopes);
}

void InfixExpr::bind(Scopes& scopes) const {
    lhs->bind(scopes);
    rhs->bind(scopes);
}

void LambdaExpr::bind(Scopes& scopes) const {
    domain->bind(scopes);
}

void PrefixExpr::bind(Scopes& scopes) const {
    rhs->bind(scopes);
}

void PostfixExpr::bind(Scopes& scopes) const {
    lhs->bind(scopes);
}

void TupleExpr::Elem::bind(Scopes& scopes) const {
    expr->bind(scopes);
}

void TupleExpr::bind(Scopes& scopes) const {
    for (auto&& elem : elems)
        elem->bind(scopes);
    type->bind(scopes);
}

void UnknownExpr::bind(Scopes&) const {}

void PackExpr::bind(Scopes& scopes) const {
    for (auto&& domain : domains)
        domain->bind(scopes);
    body->bind(scopes);
}

void SigmaExpr::bind(Scopes& scopes) const {
    for (auto&& elem : elems)
        elem->bind(scopes);
}

void VariadicExpr::bind(Scopes& scopes) const {
    for (auto&& domain : domains)
        domain->bind(scopes);
    body->bind(scopes);
}

void ErrorExpr::bind(Scopes&) const {}

/*
 * Stmnt
 */

void ExprStmnt::bind(Scopes& scopes) const {
    expr->bind(scopes);
}

void LetStmnt::bind(Scopes& scopes) const {
    if (init)
        init->bind(scopes);
    ptrn->bind(scopes);
}

}

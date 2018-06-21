#include "impala/print.h"

#include "thorin/util/array.h"

#include "impala/ast.h"

namespace impala {

using thorin::streamf;

//------------------------------------------------------------------------------

/*
 * helpers
 */

static bool is_cn_type(const Expr* expr) {
    if (auto forall = expr->isa<ForallExpr>(); forall && forall->returns_bottom())
        return true;
    return false;
}

static bool is_cn_type(const Ptrn* ptrn) { return is_cn_type(ptrn->type.get()); }

static std::optional<std::pair<const Ptrn*, const Ptrn*>> dissect_ptrn(const Ptrn* ptrn) {
    if (auto tuple = ptrn->isa<TuplePtrn>(); tuple && tuple->elems.size() == 2 && is_cn_type(tuple->elems.back().get()) && tuple->elems.back()->as<IdPtrn>()->symbol() == "return")
        return std::optional(std::pair{tuple->elems.front().get(), tuple->elems.back().get()});
    return {};
}

//------------------------------------------------------------------------------

std::ostream& Node::stream_out(std::ostream& s) const {
    Printer printer(s);
    stream(printer);
    return s;
}

//------------------------------------------------------------------------------

/*
 * misc
 */

Printer& Prg::stream(Printer& p) const {
    for (auto&& stmnt : stmnts)
        stmnt->stream(p);
    return p;
}

Printer& Id::stream(Printer& p) const {
    return streamf(p, "{}", symbol);
}

Printer& Item::stream(Printer& p) const {
    if (p.fancy()) {
        auto e = this->expr.get();
        const LambdaExpr* lambda = nullptr;
        if (auto l = e->isa<LambdaExpr>(); l && !l->returns_bottom()) {
            lambda = l;
            e = l->body.get();
        }

        if (auto f = e->isa<LambdaExpr>()) {
            if (lambda) {
                if (auto xy = dissect_ptrn(f->domain.get()))
                    return streamf(p, "fn {}[{, }]{} {}", id, lambda->domain, xy->first, f->body);
                else
                    return streamf(p, "fn {}[{, }]{} {}", id, lambda->domain, f->domain, f->body);
            } else {
                return streamf(p, "fn {}{} {}", id, f->domain, f->body);
            }
        }
    }
    return streamf(p, "letrec {} = {};", id, expr);
}

/*
 * Ptrn
 */

Printer& Ptrn::stream_ascription(Printer& p) const {
    return type->isa<UnknownExpr>() ? p : streamf(p, ": {}", type);
}

Printer& IdPtrn::stream(Printer& p) const {
    if (type_mandatory && id->symbol.is_anonymous())
        return streamf(p, "{}", type);
    streamf(p, "{}", id);
    return stream_ascription(p);
}

Printer& TuplePtrn::stream(Printer& p) const {
    streamf(p, "({, })", elems);
    return stream_ascription(p);
}

Printer& ErrorPtrn::stream(Printer& p) const {
    return streamf(p, "<error pattern>");
}

/*
 * Expr
 */

Printer& AppExpr::stream(Printer& p) const {
    if (cps) {
        if (arg->isa<TupleExpr>())
            return streamf(p, "{}{}", callee, arg);
        else
            return streamf(p, "{}({})", callee, arg);
    } else {
        if (auto tuple = arg->isa<TupleExpr>())
            return streamf(p, "{}[{, }]", callee, tuple->elems);
        else
            return streamf(p, "{}[{}]", callee, arg);
    }
}

Printer& BlockExpr::stream(Printer& p) const {
    (p << '{').indent().endl();
    for (auto&& stmnt : stmnts)
        stmnt->stream(p).endl();
    expr->stream(p).dedent().endl();
    return (p << '}').endl();
}

Printer& BottomExpr::stream(Printer& p) const {
    return streamf(p, "⊥");
}

Printer& FieldExpr::stream(Printer& p) const {
    return streamf(p, "{}.{}", lhs, id);
}

Printer& ForallExpr::stream(Printer& p) const {
    if (p.fancy() && is_cn_type(this)) {
        if (auto sigma = domain->type->isa<SigmaExpr>(); sigma && sigma->elems.size() == 2 && is_cn_type(sigma->elems.back().get()))
            return streamf(p, "Fn {} -> {}", sigma->elems.front(), sigma->elems.back()->type->as<ForallExpr>()->domain);
        return streamf(p, "Cn {}", domain);
    }
    return streamf(p, "\\/ {} -> {}", domain, codomain);
}

Printer& IdExpr::stream(Printer& p) const {
    return streamf(p, "{}", id);
}

Printer& IfExpr::stream(Printer& p) const {
    return streamf(p, "if {} {}else {}", cond, then_expr, else_expr);
}

Printer& InfixExpr::stream(Printer& p) const {
    return streamf(p, "({} {} {})", lhs, Token::tag2str((Token::Tag) tag), rhs);
}

Printer& LambdaExpr::stream(Printer& p) const {
    if (p.fancy()) {
        if (returns_bottom()) {
            if (auto xy = dissect_ptrn(domain.get()))
                return streamf(p, "fn {} {}", xy->first, body);
            return streamf(p, "cn {} {}", domain, body);
        }
        if (codomain->isa<UnknownExpr>())
            return streamf(p, "\\ {} {}", domain, body);
    }
    return streamf(p, "\\ {} -> {} {}", domain, codomain, body);
}

Printer& PrefixExpr::stream(Printer& p) const {
    return streamf(p, "({}{})", Token::tag2str((Token::Tag) tag), rhs);
}

Printer& PostfixExpr::stream(Printer& p) const {
    return streamf(p, "({}{})", lhs, Token::tag2str((Token::Tag) tag));
}

Printer& QualifierExpr::stream(Printer& p) const {
    return streamf(p, "{}", Token::tag2str((Token::Tag) tag));
}

Printer& TupleExpr::Elem::stream(Printer& p) const {
    if (p.fancy() && id->symbol.is_anonymous())
        return streamf(p, "{}", expr);
    return streamf(p, "{}= {}", id, expr);
}

Printer& TupleExpr::stream(Printer& p) const {
    if (p.fancy() && type->isa<UnknownExpr>())
        return streamf(p, "({, })", elems);
    return streamf(p, "({, }): {}", elems, type);
}

Printer& UnknownExpr::stream(Printer& p) const {
    return streamf(p, "<?>");
}

Printer& PackExpr::stream(Printer& p) const {
    return streamf(p, "pk({, }; {})", domains, body);
}

Printer& SigmaExpr::stream(Printer& p) const {
    return streamf(p, "[{, }]", elems);
}

Printer& TypeExpr::stream(Printer& p) const {
    if (auto q = qualifier->isa<QualifierExpr>(); p.fancy() && q && q->tag == QualifierExpr::Tag::u)
        return streamf(p, "type");
    return streamf(p, "type({})", qualifier);
}

Printer& VariadicExpr::stream(Printer& p) const {
    return streamf(p, "ar[{, }; {}]", domains, body);
}

Printer& ErrorExpr::stream(Printer& p) const {
    return streamf(p, "<error expression>");
}

/*
 * Stmnt
 */

Printer& ExprStmnt::stream(Printer& p) const {
    return streamf(p, "{};", expr);
}

Printer& LetStmnt::stream(Printer& p) const {
    if (init)
        return streamf(p, "let {} = {};", ptrn, init);
    else
        return streamf(p, "let {};", ptrn);
}

Printer& ItemStmnt::stream(Printer& p) const {
    return streamf(p, "{}", item).endl();
}

}

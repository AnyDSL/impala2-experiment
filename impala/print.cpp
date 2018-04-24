#include "impala/print.h"

#include "impala/ast.h"

namespace impala {

using thorin::streamf;

Printer& Printer::endl() {
    ostream() << std::endl;
    for (int i = 0; i != level_; ++i)
        ostream() << '\t';
    return *this;
}

void Node::stream(std::ostream& ostream, bool fancy) const {
    Printer printer(ostream, fancy);
    stream(printer);
}

Printer& Node::stream(Printer& p) const {
    return p << "TODO";
}

//------------------------------------------------------------------------------

/*
 * misc
 */

Printer& Id::stream(Printer& p) const {
    return streamf(p, "{}", symbol);
}

/*
 * Ptrn
 */

Printer& Ptrn::stream_ascription(Printer& p) const {
    return type ? streamf(p, ": {}", type) : p;
}

Printer& IdPtrn::stream(Printer& p) const {
    if (type_mandatory && id->symbol.is_anonymous())
        return streamf(p, "{}", type);
    streamf(p, "{}", id);
    return stream_ascription(p);
}

Printer& TuplePtrn::stream(Printer& p) const {
    streamf(p, "({, })", ptrns);
    return stream_ascription(p);
}

Printer& ErrorPtrn::stream(Printer& p) const {
    return streamf(p, "<error pattern>");
}

/*
 * Expr
 */

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

Printer& ForallExpr::stream(Printer& p) const {
    if (codomain->isa<BottomExpr>()) {
        if (auto sigma_expr = domain->isa<SigmaExpr>(); sigma_expr && !sigma_expr->elems.empty() && sigma_expr->elems.back()->type->isa<BottomExpr>())
            return streamf(p, "Fn {} -> {}", domain, "TODO");
        return streamf(p, "Cn {}", domain);
    }
    return streamf(p, "\\/ {} -> {}", domain, codomain);
}

Printer& IdExpr::stream(Printer& p) const {
    return streamf(p, "{}", id);
}

Printer& InfixExpr::stream(Printer& p) const {
    return streamf(p, "({} {} {})", lhs, Token::tag2str((Token::Tag) tag), rhs);
}

Printer& LambdaExpr::stream(Printer& p) const {
    return streamf(p, "\\ {} -> {} {}", domain, codomain, body);
}

Printer& PrefixExpr::stream(Printer& p) const {
    return streamf(p, "({}{})", Token::tag2str((Token::Tag) tag), rhs);
}

Printer& PostfixExpr::stream(Printer& p) const {
    return streamf(p, "({}{})", lhs, Token::tag2str((Token::Tag) tag));
}

Printer& TupleExpr::Elem::stream(Printer& p) const {
    if (id->symbol.is_anonymous())
        return streamf(p, "{}", expr);
    return streamf(p, "{}= {}", id, expr);
}

Printer& TupleExpr::stream(Printer& p) const {
    return streamf(p, "({, })", elems);
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

}

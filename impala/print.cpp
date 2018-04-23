#include "impala/print.h"

#include "impala/ast.h"

namespace impala {

using thorin::streamf;
using thorin::stream_list;

std::ostream& Node::stream(std::ostream& os) const {
    Printer printer(os);
    print(printer);
    return os;
}

void Node::print(std::ostream& os, bool /*fancy*/) const {
    //Printer printer(os, fancy);
    //print(printer);
    stream(os) << std::endl;
}

//------------------------------------------------------------------------------

void Node::print(Printer& p) const {
    if (p.fancy())
        thorin::outln("fancy");
    // TODO
}

std::ostream& Id::stream(std::ostream& os) const {
    return streamf(os, "{}", symbol);
}

/*
 * Ptrn
 */

std::ostream& Ptrn::stream_ascription(std::ostream& os) const {
    return type ? streamf(os, ": {}", type) : os;
}

std::ostream& IdPtrn::stream(std::ostream& os) const {
    if (type_mandatory && id->symbol.is_anonymous())
        return streamf(os, "{}", type);
    streamf(os, "{}", id);
    return stream_ascription(os);
}

std::ostream& TuplePtrn::stream(std::ostream& os) const {
    stream_list(os, ptrns, [&](auto&& ptrn) { ptrn->stream(os); }, "(", ")");
    return stream_ascription(os);
}

std::ostream& ErrorPtrn::stream(std::ostream& os) const {
    return streamf(os, "<error pattern>");
}

/*
 * Expr
 */

std::ostream& BlockExpr::stream(std::ostream& os) const {
    for (auto&& stmnt : stmnts)
        stmnt->stream(os) << std::endl;
    return expr->stream(os);
}

std::ostream& IdExpr::stream(std::ostream& os) const {
    return streamf(os, "{}", id);
}

std::ostream& InfixExpr::stream(std::ostream& os) const {
    return streamf(os, "({}{}{})", lhs, Token::tag2str((Token::Tag) tag), rhs);
}

std::ostream& PrefixExpr::stream(std::ostream& os) const {
    return streamf(os, "({}{})", Token::tag2str((Token::Tag) tag), rhs);
}

std::ostream& PostfixExpr::stream(std::ostream& os) const {
    return streamf(os, "({}{})", lhs, Token::tag2str((Token::Tag) tag));
}

std::ostream& TupleExpr::Elem::stream(std::ostream& os) const {
    if (id->symbol.is_anonymous())
        return streamf(os, "{}", expr);
    return streamf(os, "{}= {}", id, expr);
}

std::ostream& TupleExpr::stream(std::ostream& os) const {
    return stream_list(os, elems, [&](auto&& elem) { elem->stream(os); }, "(", ")");
}

std::ostream& PackExpr::stream(std::ostream& os) const {
    stream_list(os << "pk(", domains, [&](auto&& ptrn) { ptrn->stream(os); });
    return streamf(os, "; {})", body);
}

std::ostream& SigmaExpr::stream(std::ostream& os) const {
    return stream_list(os, elems, [&](auto&& elem) { elem->stream(os); }, "[", "]");
}

std::ostream& VariadicExpr::stream(std::ostream& os) const {
    stream_list(os << "ar[", domains, [&](auto&& ptrn) { ptrn->stream(os); });
    return streamf(os, "; {}]", body);
}

std::ostream& ErrorExpr::stream(std::ostream& os) const {
    return streamf(os, "<error expression>");
}

/*
 * Stmnt
 */

std::ostream& ExprStmnt::stream(std::ostream& os) const {
    return streamf(os, "{};", expr);
}

std::ostream& LetStmnt::stream(std::ostream& os) const {
    if (init)
        return streamf(os, "let {} = {};", ptrn, init);
    else
        return streamf(os, "let {};", ptrn);
}

}

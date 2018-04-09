#include "impala/parser.h"

#include <sstream>

namespace impala {

Parser::Parser(Compiler& compiler, std::istream& stream, const char* filename)
    : lexer_(compiler, stream, filename)
{
    for (int i = 0; i != max_ahead; ++i)
        lex();
    prev_ = Location(filename, 1, 1, 1, 1);
}

/*
 * helpers
 */

Token Parser::lex() {
    prev_ = ahead_[0].location();
    for (int i = 0; i < max_ahead - 1; i++)
        ahead_[i] = ahead_[i + 1];
    ahead_[max_ahead - 1] = lexer_.lex();
    return ahead();
}

bool Parser::accept(Token::Tag tag) {
    if (tag != ahead().tag())
        return false;
    lex();
    return true;
}

bool Parser::expect(Token::Tag tag, const std::string& context) {
    if (ahead().tag() == tag) {
        lex();
        return true;
    }
    std::ostringstream oss;
    thorin::streamf(oss, "'{}'", Token::tag_to_string(tag));
    error(oss.str(), context);
    return false;
}

void Parser::error(const std::string& what, const std::string& context, const Token& tok) {
    lexer_.compiler.error(tok.location(), "expected {}, got '{}'{}", what, tok,
            context.empty() ? "" : std::string(" while parsing ") + context.c_str());
}

/*
 * Expr
 */

Ptr<Expr> Parser::parse_expr() {
    return nullptr;
}

Ptr<Expr> Parser::try_block_expr() {
    if (ahead().isa(Token::Tag::D_l_brace))
        return parse_block_expr();
    return empty_expr();
}

Ptr<BlockExpr> Parser::parse_block_expr() {
    return nullptr;
}

Ptr<IfExpr> Parser::parse_if_expr() {
    auto tracker = track();
    eat(Token::Tag::K_if);
    auto cond = parse_expr();
    auto then_expr = try_block_expr();
    auto else_expr = accept(Token::Tag::K_else) ? try_block_expr() : empty_expr();
    return std::make_unique<IfExpr>(tracker, std::move(cond), std::move(then_expr), std::move(else_expr));
}

}

#ifndef IMPALA_PARSER_H
#define IMPALA_PARSER_H

#include <array>

#include "impala/ast.h"
#include "impala/lexer.h"

namespace impala {

class Compiler;

class Parser {
public:
    Parser(Compiler&, std::istream&, const char* filename);

    //@{ Expr%s
    Ptr<TupleExpr> empty_expr() { return std::make_unique<TupleExpr>(prev_); }
    Ptr<Expr> try_block_expr();
    Ptr<Expr> parse_expr();
    Ptr<BlockExpr> parse_block_expr();
    Ptr<IfExpr> parse_if_expr();
    //@}

private:
    const Token& ahead(size_t i = 0) const { assert(i < max_ahead); return ahead_[i]; }

#ifdef NDEBUG
    Token eat(Token::Tag) { return lex(); }
#else
    Token eat(Token::Tag tag) { assert(tag == ahead().tag() && "internal parser error"); return lex(); }
#endif

    bool accept(Token::Tag tok);
    bool expect(Token::Tag tok, const std::string& context);
    void error(const std::string& what, const std::string& context) { error(what, context, ahead()); }
    void error(const std::string& what, const std::string& context, const Token& tok);

    template<class F>
    void parse_list(const char* context, Token::Tag delimiter, Token::Tag sep, F f) {
        if (!ahead().isa(delimiter)) {
            do { f(); }
            while (accept(sep) && !ahead().isa(delimiter));
        }

        expect(delimiter, context);
    }

    class Tracker {
    public:
        Tracker(Parser& parser, const Location& location)
            : parser_(parser)
            , location_(location)
        {}

        operator Location() const { return {location_.front(), parser_.prev_.back()}; }

    private:
        Parser& parser_;
        Location location_;
    };

    Tracker track() { return Tracker(*this, ahead().location().front()); }
    Tracker track(const Location& location) { return Tracker(*this, location); }

    /// Consume next Token in input stream, fill look-ahead buffer, return consumed Token.
    Token lex();

    Lexer lexer_;                       ///< invoked in order to get next token
    static constexpr int max_ahead = 3; ///< maximum lookahead
    std::array<Token, max_ahead> ahead_;///< SLL look ahead
    Location prev_;
};

}

#endif

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

    //@{ Ptrn%s
    Ptr<Ptrn>       parse_ptrn();
    Ptr<IdPtrn>     parse_id_ptrn();
    Ptr<TuplePtrn>  parse_tuple_ptrn();
    //@}

    //@{ Expr%s
    Ptr<TupleExpr>  empty_expr() { return make_ptr<TupleExpr>(prev_); }
    Ptr<Expr>       parse_expr();
    Ptr<BlockExpr>  parse_block_expr();
    Ptr<BlockExpr>  try_block_expr(const std::string& context);
    Ptr<IfExpr>     parse_if_expr();
    Ptr<ForExpr>    parse_for_expr();
    Ptr<MatchExpr>  parse_match_expr();
    Ptr<TupleExpr>  parse_tuple_expr();
    Ptr<WhileExpr>  parse_while_expr();
    //@}

    //@{ Stmnt%s
    Ptr<Stmnt> parse_stmnt();
    //@}
private:
    const Token& ahead(size_t i = 0) const { assert(i < max_ahead); return ahead_[i]; }
    Token eat(Token::Tag tag) { assert_unused(tag == ahead().tag() && "internal parser error"); return lex(); }
    bool accept(Token::Tag tok);
    bool expect(Token::Tag tok, const std::string& context);
    void error(const std::string& what, const std::string& context) { error(what, context, ahead()); }
    void error(const std::string& what, const std::string& context, const Token& tok);

    template<class F>
    auto parse_list(const char* context, Token::Tag l_delim, Token::Tag r_delim, Token::Tag sep, F f) -> std::deque<decltype(f())>  {
        std::deque<decltype(f())> result;
        expect(l_delim, context);
        if (!ahead().isa(r_delim)) {
            do {
                result.emplace_back(f());
            } while (accept(sep) && !ahead().isa(r_delim));
        }

        expect(r_delim, context);
        return std::move(result);
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

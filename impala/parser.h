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

    Ptr<Id>         parse_id();

    //@{ Ptrn%s
    Ptr<Ptrn>       parse_ptrn();
    Ptr<IdPtrn>     parse_id_ptrn();
    Ptr<TuplePtrn>  parse_tuple_ptrn();
    //@}

    //@{ Expr%s
    Ptr<Expr>       parse_expr();
    Ptr<BinderExpr> parse_binder_expr();
    Ptr<BlockExpr>  parse_block_expr();
    Ptr<IdExpr>     parse_id_expr();
    Ptr<IfExpr>     parse_if_expr();
    Ptr<ForExpr>    parse_for_expr();
    Ptr<MatchExpr>  parse_match_expr();
    Ptr<Expr>       parse_sigma_or_variadic_expr();
    Ptr<Expr>       parse_tuple_or_pack_expr();
    Ptr<WhileExpr>  parse_while_expr();
    //@}

    //@{ Stmnt%s
    Ptr<Stmnt>      parse_stmnt();
    Ptr<LetStmnt>   parse_let_stmnt();
    //@}
private:
    //@{ try to parse a Node
    Ptr<BlockExpr>  try_block_expr(const std::string& context);
    Ptr<Expr>       try_expr(const std::string& context);
    Ptr<Id>         try_id(const std::string& context);
    Ptr<Ptrn>       try_ptrn(const std::string& context);
    //@}

    //@{ make empty Node
    Ptr<BlockExpr>  make_empty_block_expr() { return make_ptr<BlockExpr>(prev_, Ptrs<Stmnt>{}, make_unit_expr()); }
    Ptr<Id>         make_anonymous_id() { return make_ptr<Id>(Token(prev_, "_")); }
    Ptr<TupleExpr>  make_unit_expr() { return make_ptr<TupleExpr>(prev_); }
    //@}

    const Token& ahead(size_t i = 0) const { assert(i < max_ahead); return ahead_[i]; }
    Token eat(Token::Tag tag) { assert_unused(tag == ahead().tag() && "internal parser error"); return lex(); }
    bool accept(Token::Tag tok);
    bool expect(Token::Tag tok, const std::string& context);
    void error(const std::string& what, const std::string& context) { error(what, context, ahead()); }
    void error(const std::string& what, const std::string& context, const Token& tok);

    template<class T, class F>
    void parse_list(T& ts, const char* context, Token::Tag r_delim, F f, Token::Tag sep = Token::Tag::P_comma) {
        if (!ahead().isa(r_delim)) {
            do {
                ts.emplace_back(f());
            } while (accept(sep) && !ahead().isa(r_delim));
        }
        expect(r_delim, context);
    }
    template<class F>
    auto parse_list(const char* context, Token::Tag l_delim, Token::Tag r_delim,
                    F f, Token::Tag sep = Token::Tag::P_comma) -> std::deque<decltype(f())>  {
        std::deque<decltype(f())> result;
        eat(l_delim);
        parse_list(result, context, r_delim, f, sep);
        return result;
    }
    template<class T> Ptr<Expr> parse_enclosing_expr();

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

Ptr<Expr> parse(Compiler& compiler, std::istream& is, const char* filename);
Ptr<Expr> parse(Compiler& compiler, const char*);

}

#endif

#ifndef IMPALA_PARSER_H
#define IMPALA_PARSER_H

#include <array>

#include "impala/ast.h"
#include "impala/lexer.h"

namespace impala {

class Compiler;

class Parser {
private:
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

public:
    Parser(Compiler&, std::istream&, const char* filename);

    //@{ misc
    Ptr<Id>         parse_id();
    Ptr<Expr>       parse_type_ascription(const char* ascription_context = nullptr);
    //@}

    //@{ Ptrn%s
    /**
     * @p ascription_context
     * If @c nullptr the type ascription @c :e is optional.
     * Otherwise, it is mandatory resulting in the given error message if not present.
     */
    ///other
    Ptr<Ptrn>       parse_ptrn(const char* ascription_context = nullptr);
    Ptr<IdPtrn>     parse_id_ptrn(const char* ascription_context = nullptr);
    Ptr<TuplePtrn>  parse_tuple_ptrn(const char* ascription_context = nullptr);
    //@}

    //@{ Expr%s
    Ptr<Expr>         parse_expr() { return parse_expr(Token::Prec::Bottom); }
    Ptr<Expr>         parse_expr(Token::Prec);
    Ptr<Expr>         parse_primary_expr();
    Ptr<Expr>         parse_prefix_expr();
    Ptr<Expr>         parse_postfix_expr(Tracker, Ptr<Expr>&&);
    Ptr<Expr>         parse_infix_expr(Tracker, Ptr<Expr>&&);
    //@}

    //@{ primary Expr%s
    Ptr<BlockExpr>    parse_block_expr();
    Ptr<BottomExpr>   parse_bottom_expr();
    Ptr<ForExpr>      parse_for_expr();
    Ptr<IdExpr>       parse_id_expr();
    Ptr<IfExpr>       parse_if_expr();
    Ptr<MatchExpr>    parse_match_expr();
    Ptr<PackExpr>     parse_pack_expr();
    Ptr<SigmaExpr>    parse_sigma_expr();
    Ptr<TupleExpr>    parse_tuple_expr();
    Ptr<VariadicExpr> parse_variadic_expr();
    Ptr<WhileExpr>    parse_while_expr();
    //@}

    //@{ ForallExpr%s
    Ptr<ForallExpr>   parse_cn_type_expr();
    Ptr<ForallExpr>   parse_fn_type_expr();
    Ptr<ForallExpr>   parse_forall_expr();
    //@}

    //@{ LambdaExpr%s
    Ptr<LambdaExpr>   parse_cn_expr();
    Ptr<LambdaExpr>   parse_fn_expr();
    Ptr<LambdaExpr>   parse_lambda_expr();
    //@}

    //@{ Stmnt%s
    Ptr<Stmnt>      parse_stmnt();
    Ptr<LetStmnt>   parse_let_stmnt();
    //@}

private:
    //@{ try to parse a Node
    Ptr<BlockExpr>  try_block_expr(const char* context);
    Ptr<Expr>       try_expr(const char* context);
    Ptr<Id>         try_id(const char* context);
    Ptr<Ptrn>       try_ptrn(const char* context);
    Ptr<Ptrn>       try_ptrn_t(const char* ascription_context = nullptr);
    //@}

    //@{ make AST nodes
    Ptr<BottomExpr>   make_bottom_expr()      { return make_ptr<BottomExpr> (prev_); }
    Ptr<BlockExpr>    make_empty_block_expr() { return make_ptr<BlockExpr>  (prev_, Ptrs<Stmnt>{}, make_unit_tuple()); }
    Ptr<ErrorExpr>    make_error_expr()       { return make_ptr<ErrorExpr>  (prev_); }
    Ptr<TupleExpr>    make_unit_tuple()       { return make_ptr<TupleExpr>  (prev_, Ptrs<TupleExpr::Elem>{}, Ptr<Expr>{}); }
    Ptr<UnknownExpr>  make_unknown_expr()     { return make_ptr<UnknownExpr>(prev_); }

    Ptr<TupleExpr::Elem> make_tuple_elem(Ptr<Expr>&& expr) {
        auto location = expr->location;
        return make_ptr<TupleExpr::Elem>(location, make_ptr<Id>(Token(location, Symbol("_"))), std::move(expr));
    }
    Ptr<TupleExpr>    make_tuple(Ptr<Expr>&& lhs, Ptr<Expr>&& rhs) {
        Ptrs<TupleExpr::Elem> args;
        auto location = lhs->location + rhs->location;
        args.emplace_back(make_tuple_elem(std::move(lhs)));
        args.emplace_back(make_tuple_elem(std::move(rhs)));
        return make_ptr<TupleExpr>(location, std::move(args), make_unknown_expr());
    }
    Ptr<Id>           make_id(const char* s)  { return make_ptr<Id>(Token(prev_, s)); }
    Ptr<IdPtrn>       make_id_ptrn(const char* s, Ptr<Expr>&& type) {
        auto location = type->location;
        return make_ptr<IdPtrn>(location, make_id(s), std::move(type), true);
    }
    Ptr<ForallExpr>   make_cn_type(Ptr<Ptrn>&& domain) {
        auto location = domain->location;
        return make_ptr<ForallExpr>(location, std::move(domain), make_bottom_expr());
    }
    //@}

    const Token& ahead(size_t i = 0) const { assert(i < max_ahead); return ahead_[i]; }
    Token eat(Token::Tag tag) { assert_unused(tag == ahead().tag() && "internal parser error"); return lex(); }
    bool accept(Token::Tag tok);
    bool expect(Token::Tag tok, const char* context);
    void error(const char* what, const char* context) { error(what, ahead(), context); }
    void error(const char* what, const Token& tok, const char* context);

    template<class F>
    auto parse_list(Token::Tag delim_r, F f, Token::Tag sep = Token::Tag::P_comma) -> std::deque<decltype(f())> {
        std::deque<decltype(f())> result;
        if (!ahead().isa(delim_r)) {
            do {
                result.emplace_back(f());
            } while (accept(sep) && !ahead().isa(delim_r));
        }
        return result;
    }
    template<class F>
    auto parse_list(const char* context, Token::Tag delim_l, Token::Tag delim_r,
                    F f, Token::Tag sep = Token::Tag::P_comma) -> std::deque<decltype(f())>  {
        eat(delim_l);
        auto result = parse_list(delim_r, f, sep);
        expect(delim_r, context);
        return result;
    }

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

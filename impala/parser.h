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
        Tracker(Parser& parser, const Loc& loc)
            : parser_(parser)
            , loc_(loc)
        {}

        operator Loc() const { return {loc_.front(), parser_.prev_.back()}; }

    private:
        Parser& parser_;
        Loc loc_;
    };

public:
    Parser(Compiler&, std::istream&, const char* filename);

    Compiler& compiler() { return lexer_.compiler; }

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
    Ptr<TuplePtrn>  parse_tuple_ptrn(const char* ascription_context = nullptr, TT delim_l = TT::D_paren_l, TT delim_r = TT::D_paren_r);
    //@}

    //@{ Expr%s
    Ptr<Expr>         parse_expr(Token::Prec = Token::Prec::Bottom);
    Ptr<Expr>         parse_prefix_expr();
    Ptr<Expr>         parse_infix_expr(Tracker, Ptr<Expr>&&);
    Ptr<Expr>         parse_postfix_expr(Tracker, Ptr<Expr>&&);
    Ptr<AppExpr>      parse_cps_app_expr(Tracker, Ptr<Expr>&&);
    Ptr<AppExpr>      parse_ds_app_expr(Tracker, Ptr<Expr>&&);
    Ptr<FieldExpr>    parse_field_expr(Tracker, Ptr<Expr>&&);
    //@}

    //@{ primary Expr%s
    Ptr<Expr>         parse_primary_expr();
    Ptr<BlockExpr>    parse_block_expr();
    Ptr<BottomExpr>   parse_bottom_expr();
    Ptr<ForExpr>      parse_for_expr();
    Ptr<IdExpr>       parse_id_expr();
    Ptr<IfExpr>       parse_if_expr();
    Ptr<MatchExpr>    parse_match_expr();
    Ptr<PackExpr>     parse_pack_expr();
    Ptr<SigmaExpr>    parse_sigma_expr();
    Ptr<TupleExpr>    parse_tuple_expr(TT delim_l = TT::D_paren_l, TT delim_r = TT::D_paren_r);
    Ptr<VariadicExpr> parse_variadic_expr();
    Ptr<WhileExpr>    parse_while_expr();
    //@}

    //@{ ForallExpr%s
    Ptr<ForallExpr>   parse_cn_type_expr();
    Ptr<ForallExpr>   parse_fn_type_expr();
    Ptr<ForallExpr>   parse_forall_expr();
    //@}

    //@{ LambdaExpr%s
    Ptr<LambdaExpr>   parse_cn_expr(bool item = false);
    Ptr<LambdaExpr>   parse_fn_expr(bool item = false);
    Ptr<LambdaExpr>   parse_lambda_expr();
    //@}

    //@{ Stmnt%s
    Ptr<Stmnt>      parse_stmnt();
    Ptr<LetStmnt>   parse_let_stmnt();
    Ptr<ItemStmnt>  parse_item_stmnt();
    //@}

private:
    //@{ try to parse a Node
    Ptr<BlockExpr>  try_block_expr(const char* context);
    Ptr<Expr>       try_expr(const char* context, Token::Prec prec = Token::Prec::Bottom);
    Ptr<Id>         try_id(const char* context);
    Ptr<Ptrn>       try_ptrn(const char* context);
    Ptr<TuplePtrn>  try_tuple_ptrn(const char* context, TT delim_l = TT::D_paren_l, TT delim_r = TT::D_paren_r);
    /// May also be an @p Expr which is intererpreted as an @p IdPtrn with an anonymous @p Id.
    /// If @p ascription_context is not a @c nullptr the type ascription is mandatory.
    /// Otherwise, it's optional.
    Ptr<Ptrn>       try_ptrn_t(const char* ascription_context = nullptr);
    //@}

    //@{ make AST nodes
    Ptr<BottomExpr>   make_bottom_expr()      { return make_ptr<BottomExpr> (prev_); }
    Ptr<BlockExpr>    make_empty_block_expr() { return make_ptr<BlockExpr>  (prev_, Ptrs<Stmnt>{}, make_unit_tuple()); }
    Ptr<ErrorExpr>    make_error_expr()       { return make_ptr<ErrorExpr>  (prev_); }
    Ptr<TupleExpr>    make_unit_tuple()       { return make_ptr<TupleExpr>  (prev_, Ptrs<TupleExpr::Elem>{}, make_unknown_expr()); }
    Ptr<UnknownExpr>  make_unknown_expr()     { return make_ptr<UnknownExpr>(prev_); }

    Ptr<TupleExpr::Elem> make_tuple_elem(Ptr<Expr>&& expr) {
        auto loc = expr->loc;
        return make_ptr<TupleExpr::Elem>(loc, make_ptr<Id>(Token(loc, Symbol("_"))), std::move(expr));
    }
    Ptr<TupleExpr>    make_tuple(Ptr<Expr>&& lhs, Ptr<Expr>&& rhs) {
        auto loc = lhs->loc + rhs->loc;
        auto args = make_ptrs<TupleExpr::Elem>(make_tuple_elem(std::move(lhs)), make_tuple_elem(std::move(rhs)));
        return make_ptr<TupleExpr>(loc, std::move(args), make_unknown_expr());
    }
    Ptr<Id>           make_id(const char* s)  { return make_ptr<Id>(Token(prev_, s)); }
    Ptr<IdPtrn>       make_id_ptrn(const char* s, Ptr<Expr>&& type) {
        auto loc = type->loc;
        return make_ptr<IdPtrn>(loc, make_id(s), std::move(type), true);
    }
    Ptr<IdPtrn>       make_id_ptrn(Ptr<Id>&& id) {
        auto loc = id->loc;
        return make_ptr<IdPtrn>(loc, std::move(id), make_unknown_expr(), false);
    }
    Ptr<ForallExpr>   make_cn_type(Ptr<Ptrn>&& domain) {
        auto loc = domain->loc;
        return make_ptr<ForallExpr>(loc, std::move(domain), make_bottom_expr());
    }
    //@}

    const Token& ahead(size_t i = 0) const { assert(i < max_ahead); return ahead_[i]; }
    Token eat(TT tag) { assert_unused(tag == ahead().tag() && "internal parser error"); return lex(); }
    bool accept(TT tok);
    bool expect(TT tok, const char* context);
    void error(const char* what, const char* context) { error(what, ahead(), context); }
    void error(const char* what, const Token& tok, const char* context);

    template<class F>
    auto parse_list(TT delim_r, F f, TT sep = TT::P_comma) -> std::deque<decltype(f())> {
        std::deque<decltype(f())> result;
        if (!ahead().isa(delim_r)) {
            do {
                result.emplace_back(f());
            } while (accept(sep) && !ahead().isa(delim_r));
        }
        return result;
    }
    template<class F>
    auto parse_list(const char* context, TT delim_l, TT delim_r, F f, TT sep = TT::P_comma) -> std::deque<decltype(f())>  {
        eat(delim_l);
        auto result = parse_list(delim_r, f, sep);
        expect(delim_r, context);
        return result;
    }

    Tracker track() { return Tracker(*this, ahead().loc().front()); }
    Tracker track(const Loc& loc) { return Tracker(*this, loc); }

    /// Consume next Token in input stream, fill look-ahead buffer, return consumed Token.
    Token lex();

    Lexer lexer_;                       ///< invoked in order to get next token
    static constexpr int max_ahead = 3; ///< maximum lookahead
    std::array<Token, max_ahead> ahead_;///< SLL look ahead
    Loc prev_;
};

Ptr<Expr> parse(Compiler& compiler, std::istream& is, const char* filename);
Ptr<Expr> parse(Compiler& compiler, const char*);

}

#endif

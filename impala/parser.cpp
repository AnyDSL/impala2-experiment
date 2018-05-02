#include "impala/parser.h"

#include <sstream>

#define EXPR \
         Token::Tag::D_brace_l: \
    case Token::Tag::D_bracket_l: \
    case Token::Tag::D_paren_l: \
    case Token::Tag::K_ar: \
    case Token::Tag::K_Cn: \
    case Token::Tag::K_Fn: \
    case Token::Tag::K_cn: \
    case Token::Tag::K_false: \
    case Token::Tag::K_fn: \
    case Token::Tag::K_for: \
    case Token::Tag::K_if: \
    case Token::Tag::K_match: \
    case Token::Tag::K_pk: \
    case Token::Tag::K_true: \
    case Token::Tag::K_while: \
    case Token::Tag::L_f: \
    case Token::Tag::L_s: \
    case Token::Tag::L_u: \
    case Token::Tag::M_id: \
    case Token::Tag::O_lambda: \
    case Token::Tag::O_forall: \
    case Token::Tag::O_add: \
    case Token::Tag::O_and: \
    case Token::Tag::O_dec: \
    case Token::Tag::O_inc: \
    case Token::Tag::O_mul: \
    case Token::Tag::O_not: \
    case Token::Tag::O_sub: \
    case Token::Tag::O_tilde

namespace impala {

Parser::Parser(Compiler& compiler, std::istream& stream, const char* filename)
    : lexer_(compiler, stream, filename)
{
    for (int i = 0; i != max_ahead; ++i) lex();
    prev_ = Location(filename, 1, 1, 1, 1);
}

/*
 * helpers
 */

Token Parser::lex() {
    auto result = ahead();
    prev_ = ahead_[0].location();
    for (int i = 0; i < max_ahead - 1; i++)
        ahead_[i] = ahead_[i + 1];
    ahead_[max_ahead - 1] = lexer_.lex();
    return result;
}

bool Parser::accept(Token::Tag tag) {
    if (tag != ahead().tag())
        return false;
    lex();
    return true;
}

bool Parser::expect(Token::Tag tag, const char* context) {
    if (ahead().tag() == tag) {
        lex();
        return true;
    }
    std::ostringstream os;
    thorin::streamf(os, "'{}'", Token::tag2str(tag));
    error(os.str().c_str(), context);
    return false;
}

void Parser::error(const char* what, const Token& tok, const char* context) {
    lexer_.compiler.error(tok.location(), "expected {}, got '{}' while parsing {}", what, tok, context);
}

/*
 * try
 */

Ptr<Expr> Parser::try_expr(const char* context) {
    switch (ahead().tag()) {
        case EXPR: return parse_expr();
        default: break;
    }
    error("expression", context);
    return make_ptr<ErrorExpr>(prev_);
}

Ptr<Id> Parser::try_id(const char* context) {
    if (ahead().isa(Token::Tag::M_id)) return parse_id();
    error("identifier", context);
    return make_ptr<Id>(Token(prev_, "<error>"));
}

Ptr<Ptrn> Parser::try_ptrn(const char* context) {
    switch (ahead().tag()) {
        case Token::Tag::M_id:
        case Token::Tag::D_paren_l: return parse_ptrn();
        default: break;
    }

    error("pattern", context);
    return make_ptr<ErrorPtrn>(prev_);
}

Ptr<Ptrn> Parser::try_ptrn_t(const char* ascription_context) {
    if ((ahead(0).isa(Token::Tag::M_id) && ahead(1).isa(Token::Tag::P_colon)) // IdPtrn
            || ahead().isa(Token::Tag::D_paren_l)) {                          // TuplePtrn
        return parse_ptrn(ascription_context);
    }
    auto tracker = track();
    auto type = try_expr(ascription_context);
    return make_ptr<IdPtrn>(tracker, make_id("_"), std::move(type), true);
}

Ptr<BlockExpr> Parser::try_block_expr(const char* context) {
    if (ahead().isa(Token::Tag::D_brace_l)) return parse_block_expr();
    error("block expression", context);
    return make_empty_block_expr();
}

/*
 * misc
 */

Ptr<Id> Parser::parse_id() { return make_ptr<Id>(eat(Token::Tag::M_id)); }

Ptr<Expr> Parser::parse_type_ascription(const char* ascription_context) {
    if (ascription_context) {
        expect(Token::Tag::P_colon, ascription_context);
        return try_expr("type ascription");
    }

    return accept(Token::Tag::P_colon) ? try_expr("type ascription") : make_unknown_expr();
}

/*
 * Ptrn
 */

Ptr<Ptrn> Parser::parse_ptrn(const char* ascription_context) {
    Ptr<Ptrn> ptrn;
    switch (ahead().tag()) {
        case Token::Tag::M_id:      return parse_id_ptrn(ascription_context);
        case Token::Tag::D_paren_l: return parse_tuple_ptrn(ascription_context);
        default: THORIN_UNREACHABLE;
    }
}

Ptr<IdPtrn> Parser::parse_id_ptrn(const char* ascription_context) {
    auto tracker = track();
    auto id = parse_id();
    auto type = parse_type_ascription(ascription_context);
    return make_ptr<IdPtrn>(tracker, std::move(id), std::move(type), bool(ascription_context));
}

Ptr<TuplePtrn> Parser::parse_tuple_ptrn(const char* ascription_context) {
    auto tracker = track();
    auto ptrns = parse_list("tuple pattern", Token::Tag::D_paren_l, Token::Tag::D_paren_r, [&]{ return try_ptrn("sub-pattern of a tuple pattern"); });
    auto type = parse_type_ascription(ascription_context);
    return make_ptr<TuplePtrn>(tracker, std::move(ptrns), std::move(type), bool(ascription_context));
}

/*
 * Expr - prefix, infix, postfix
 */

Ptr<Expr> Parser::parse_expr(Token::Prec p) {
    auto tracker = track();
    auto lhs = parse_primary_expr();

    while (true) {
        switch (ahead().tag()) {
            case Token::Tag::P_dot:       lhs = parse_field_expr(tracker, std::move(lhs));   continue;
            case Token::Tag::D_paren_l:   lhs = parse_cps_app_expr(tracker, std::move(lhs)); continue;
            case Token::Tag::D_bracket_l: lhs = parse_ds_app_expr(tracker, std::move(lhs));  continue;
            case Token::Tag::O_inc:
            case Token::Tag::O_dec:       lhs = parse_postfix_expr(tracker, std::move(lhs)); continue;
            default: break;
        }

        if (auto q = Token::tag2prec(ahead().tag()); p < q)
            lhs = parse_infix_expr(tracker, std::move(lhs));
        else
            break;
    }

    return lhs;
}

Ptr<Expr> Parser::parse_prefix_expr() {
    auto tracker = track();
    auto tag = lex().tag();
    auto rhs = parse_expr(Token::Prec::Unary);

    return make_ptr<PrefixExpr>(tracker, (PrefixExpr::Tag) tag, std::move(rhs));
}

Ptr<Expr> Parser::parse_infix_expr(Tracker tracker, Ptr<Expr>&& lhs) {
    auto token = lex();
    auto rhs = parse_expr(Token::tag2prec(token.tag()));
    if (auto name = Token::tag2name(token.tag()); name[0] != '\0') {
        auto callee = make_ptr<IdExpr>(make_ptr<Id>(Token(token.location(), Symbol(name))));
        auto args = make_tuple(std::move(lhs), std::move(rhs));
        return make_ptr<AppExpr>(tracker, std::move(callee), std::move(args), true);
    }
    return make_ptr<InfixExpr>(tracker, std::move(lhs), (InfixExpr::Tag) token.tag(), std::move(rhs));
}

Ptr<Expr> Parser::parse_postfix_expr(Tracker tracker, Ptr<Expr>&& lhs) {
    auto tag = lex().tag();
    return make_ptr<PostfixExpr>(tracker, std::move(lhs), (PostfixExpr::Tag) tag);
}

Ptr<AppExpr> Parser::parse_cps_app_expr(Tracker tracker, Ptr<Expr>&& callee) {
    auto arg = parse_tuple_expr();
    return make_ptr<AppExpr>(tracker, std::move(callee), std::move(arg), true);
}

Ptr<AppExpr> Parser::parse_ds_app_expr(Tracker tracker, Ptr<Expr>&& callee) {
    auto arg = parse_tuple_expr(Token::Tag::D_bracket_l, Token::Tag::D_bracket_r);
    return make_ptr<AppExpr>(tracker, std::move(callee), std::move(arg), false);
}

Ptr<FieldExpr> Parser::parse_field_expr(Tracker tracker, Ptr<Expr>&& lhs) {
    eat(Token::Tag::P_dot);
    auto id = try_id("field expression");
    return make_ptr<FieldExpr>(tracker, std::move(lhs), std::move(id));
}

/*
 * primary expr
 */

Ptr<Expr> Parser::parse_primary_expr() {
    switch (ahead().tag()) {
        case Token::Tag::O_inc:
        case Token::Tag::O_dec:
        case Token::Tag::O_add:
        case Token::Tag::O_sub:       return parse_prefix_expr();
        case Token::Tag::D_brace_l:   return parse_block_expr();
        case Token::Tag::D_bracket_l: return parse_sigma_expr();
        case Token::Tag::D_paren_l:   return parse_tuple_expr();
        case Token::Tag::K_ar:        return parse_variadic_expr();
        case Token::Tag::K_cn:        return parse_cn_expr();
        case Token::Tag::K_Cn:        return parse_cn_type_expr();
        case Token::Tag::K_fn:        return parse_fn_expr();
        case Token::Tag::K_Fn:        return parse_fn_type_expr();
        case Token::Tag::K_if:        return parse_if_expr();
        case Token::Tag::K_pk:        return parse_pack_expr();
        case Token::Tag::M_id:        return parse_id_expr();
        case Token::Tag::O_forall:    return parse_forall_expr();
        case Token::Tag::O_lambda:    return parse_lambda_expr();
        default:
            error("expression", "primary expression");
            return make_error_expr();
    }
}

Ptr<BlockExpr> Parser::parse_block_expr() {
    auto tracker = track();
    eat(Token::Tag::D_brace_l);
    Ptrs<Stmnt> stmnts;
    Ptr<Expr> final_expr;
    while (true) {
        switch (ahead().tag()) {
            case Token::Tag::P_semicolon: lex(); continue; // ignore semicolon
            //case ITEM:             stmnts.emplace_back(parse_item_stmnt()); continue;
            case Token::Tag::K_let:       stmnts.emplace_back(parse_let_stmnt()); continue;
            //case Token::ASM:       stmnts.emplace_back(parse_asm_stmnt()); continue;
            case EXPR: {
                auto tracker = track();
                Ptr<Expr> expr;
                bool stmnt_like = true;
                switch (ahead().tag()) {
                    case Token::Tag::K_if:      expr = parse_if_expr(); break;
                    case Token::Tag::K_match:   expr = parse_match_expr(); break;
                    case Token::Tag::K_for:     expr = parse_for_expr(); break;
                    case Token::Tag::K_while:   expr = parse_while_expr(); break;
                    case Token::Tag::D_brace_l: expr = parse_block_expr(); break;
                    default:                    expr = parse_expr(); stmnt_like = false;
                }

                if (accept(Token::Tag::P_semicolon) || (stmnt_like && !ahead().isa(Token::Tag::D_brace_r))) {
                    stmnts.emplace_back(make_ptr<ExprStmnt>(tracker, std::move(expr)));
                    continue;
                }

                swap(final_expr, expr);
                [[fallthrough]];
            }
            default:
                expect(Token::Tag::D_brace_r, "block expression");
                if (!final_expr)
                    final_expr = make_unit_tuple();
                return make_ptr<BlockExpr>(tracker, std::move(stmnts), std::move(final_expr));
        }
    }
}

Ptr<IdExpr> Parser::parse_id_expr() {
    return make_ptr<IdExpr>(parse_id());
}

Ptr<IfExpr> Parser::parse_if_expr() {
    auto tracker = track();
    eat(Token::Tag::K_if);
    auto cond = parse_expr();
    auto then_expr = try_block_expr("consequence of an if expression");
    Ptr<Expr> else_expr;
    if (accept(Token::Tag::K_else)) {
        switch (ahead().tag()) {
            case Token::Tag::K_if:      else_expr = parse_if_expr(); break;
            case Token::Tag::D_brace_l: else_expr = parse_block_expr(); break;
            default: error("block or if expression", "alternative of an if expression");
        }
    }

    if (!else_expr)
        else_expr = make_empty_block_expr();

    return make_ptr<IfExpr>(tracker, std::move(cond), std::move(then_expr), std::move(else_expr));
}

Ptr<ForExpr> Parser::parse_for_expr() {
    return nullptr;
}

Ptr<MatchExpr> Parser::parse_match_expr() {
    return nullptr;
}

Ptr<TupleExpr> Parser::parse_tuple_expr(Token::Tag delim_l, Token::Tag delim_r) {
    auto tracker = track();

    auto elems = parse_list("tuple", delim_l, delim_r, [&]{
        auto tracker = track();
        Ptr<Id> id;
        if (ahead(0).isa(Token::Tag::M_id) && ahead(1).isa(Token::Tag::O_assign)) {
            id = parse_id();
            eat(Token::Tag::O_assign);
        } else {
            id = make_id("_");
        }
        auto expr = try_expr("tuple element");
        return make_ptr<TupleExpr::Elem>(tracker, std::move(id), std::move(expr));
    });

    auto type = parse_type_ascription();
    return make_ptr<TupleExpr>(tracker, std::move(elems), std::move(type));
}

Ptr<SigmaExpr> Parser::parse_sigma_expr() {
    auto tracker = track();
    auto elems = parse_list("sigma", Token::Tag::D_bracket_l, Token::Tag::D_bracket_r, [&]{ return try_ptrn_t("type ascription of a sigma element"); });
    return make_ptr<SigmaExpr>(tracker, std::move(elems));
}

Ptr<PackExpr> Parser::parse_pack_expr() {
    auto tracker = track();
    eat(Token::Tag::K_pk);
    expect(Token::Tag::D_paren_l, "opening delimiter of a pack");
    auto domains = parse_list(Token::Tag::P_semicolon, [&]{ return try_ptrn_t("type ascription of a pack's domain"); });
    expect(Token::Tag::P_semicolon, "pack");
    auto body = try_expr("body of a pack");
    expect(Token::Tag::D_paren_r, "closing delimiter of a pack");

    return make_ptr<PackExpr>(tracker, std::move(domains), std::move(body));
}

Ptr<VariadicExpr> Parser::parse_variadic_expr() {
    auto tracker = track();
    eat(Token::Tag::K_ar);
    expect(Token::Tag::D_bracket_l, "opening delimiter of a variadic");
    auto domains = parse_list(Token::Tag::P_semicolon, [&]{ return try_ptrn_t("type ascription of a variadic's domain"); });
    expect(Token::Tag::P_semicolon, "variadic");
    auto body = try_expr("body of a variadic");
    expect(Token::Tag::D_bracket_r, "closing delimiter of a variadic");

    return make_ptr<VariadicExpr>(tracker, std::move(domains), std::move(body));
}

Ptr<WhileExpr> Parser::parse_while_expr() {
    return nullptr;
}

/*
 * LambdaExprs
 */

Ptr<LambdaExpr> Parser::parse_cn_expr() {
    auto tracker = track();
    eat(Token::Tag::K_cn);
    auto domain = try_ptrn_t();
    auto body = try_expr("body of an abstraction");

    return make_ptr<LambdaExpr>(tracker, std::move(domain), make_bottom_expr(), std::move(body));
}

Ptr<LambdaExpr> Parser::parse_fn_expr() {
    auto tracker = track();
    eat(Token::Tag::K_fn);
    auto domain = try_ptrn_t();
    auto ret = accept(Token::Tag::O_arrow) ? try_expr("codomain of an abstraction") : make_unknown_expr();
    // "_: \/ _: ret -> ⊥"
    auto ret_ptrn = make_id_ptrn("return", make_cn_type(make_id_ptrn("_", std::move(ret))));

    if (auto tuple_ptrn = domain->isa<TuplePtrn>()) {
        tuple_ptrn->elems.emplace_back(std::move(ret_ptrn));
    } else if (auto id_ptrn = domain->isa<IdPtrn>(); id_ptrn && id_ptrn->type->isa<SigmaExpr>()) {
        id_ptrn->type->as<SigmaExpr>()->elems.emplace_back(std::move(ret_ptrn));
    } else {
        auto first = std::move(domain);
        auto location = first->location + ret_ptrn->location;
        Ptrs<Ptrn> elems;
        elems.emplace_back(std::move(first));
        elems.emplace_back(std::move(ret_ptrn));
        domain = make_id_ptrn("_", make_ptr<SigmaExpr>(location, std::move(elems)));
    }

    auto body = try_expr("body of an abstraction");

    return make_ptr<LambdaExpr>(tracker, std::move(domain), make_bottom_expr(), std::move(body));
}

Ptr<LambdaExpr> Parser::parse_lambda_expr() {
    auto tracker = track();
    eat(Token::Tag::O_lambda);
    auto domain = try_ptrn("domain of an abstraction");
    auto codomain = accept(Token::Tag::O_arrow) ? try_expr("codomain of an abstraction") : make_unknown_expr();
    auto body = try_expr("body of an abstraction");

    return make_ptr<LambdaExpr>(tracker, std::move(domain), std::move(codomain), std::move(body));
}

/*
 * ForallExpr
 */

Ptr<ForallExpr> Parser::parse_cn_type_expr() {
    auto tracker = track();
    eat(Token::Tag::K_Cn);
    auto domain = try_ptrn_t();

    return make_ptr<ForallExpr>(tracker, std::move(domain), make_bottom_expr());
}

Ptr<ForallExpr> Parser::parse_fn_type_expr() {
    auto tracker = track();
    eat(Token::Tag::K_Fn);
    auto domain = try_ptrn_t();
    expect(Token::Tag::O_arrow, "function type");
    auto ret = try_expr("codomain of a function type");
    // "_: \/ _: ret -> ⊥"
    auto ret_ptrn = make_id_ptrn("_", make_cn_type(make_id_ptrn("_", std::move(ret))));

    if (auto tuple_ptrn = domain->isa<TuplePtrn>()) {
        tuple_ptrn->elems.emplace_back(std::move(ret_ptrn));
    } else if (auto id_ptrn = domain->isa<IdPtrn>(); id_ptrn && id_ptrn->type->isa<SigmaExpr>()) {
        id_ptrn->type->as<SigmaExpr>()->elems.emplace_back(std::move(ret_ptrn));
    } else {
        auto first = std::move(domain);
        auto location = first->location + ret_ptrn->location;
        Ptrs<Ptrn> elems;
        elems.emplace_back(std::move(first));
        elems.emplace_back(std::move(ret_ptrn));
        domain = make_id_ptrn("_", make_ptr<SigmaExpr>(location, std::move(elems)));
    }

    return make_ptr<ForallExpr>(tracker, std::move(domain), make_bottom_expr());
}

Ptr<ForallExpr> Parser::parse_forall_expr() {
    auto tracker = track();
    eat(Token::Tag::O_forall);
    auto domain = try_ptrn_t();
    expect(Token::Tag::O_arrow, "for-all type");
    auto codomain = try_expr("codomain of a for-all type");

    return make_ptr<ForallExpr>(tracker, std::move(domain), std::move(codomain));
}

/*
 * Stmnt
 */

Ptr<LetStmnt> Parser::parse_let_stmnt() {
    auto tracker = track();
    eat(Token::Tag::K_let);
    auto ptrn = try_ptrn("let statement");
    Ptr<Expr> init;
    if (accept(Token::Tag::O_assign))
        init = try_expr("initialization expression of a let statement");
    return make_ptr<LetStmnt>(tracker, std::move(ptrn), std::move(init));
}

//------------------------------------------------------------------------------

Ptr<Expr> parse(Compiler& compiler, std::istream& is, const char* filename) {
    Parser parser(compiler, is, filename);
    return parser.parse_expr();
}

Ptr<Expr> parse(Compiler& compiler, const char* str) {
    std::istringstream in(str);
    return parse(compiler, in, "<inline>");
}

}

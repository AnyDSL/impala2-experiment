#ifndef IMPALA_TOKEN_H
#define IMPALA_TOKEN_H

#include "thorin/util/location.h"
#include "thorin/util/symbol.h"
#include "thorin/util/types.h"
#include "thorin/util/utility.h"

namespace impala {

using thorin::Box;
using thorin::Location;
using thorin::Symbol;
using namespace thorin::literals;

#define IMPALA_KEYWORDS(f) \
    f(K_ar,     "ar") \
    f(K_Cn,     "Cn") \
    f(K_cn,     "cn") \
    f(K_else,   "else") \
    f(K_false,  "false") \
    f(K_fn,     "fn") \
    f(K_Fn,     "Fn") \
    f(K_for,    "for") \
    f(K_if,     "if") \
    f(K_impl,   "impl") \
    f(K_let,    "let") \
    f(K_match,  "match") \
    f(K_mut,    "mut") \
    f(K_pk,     "pk") \
    f(K_self,   "Self") \
    f(K_struct, "struct") \
    f(K_true,   "true") \
    f(K_trait,  "trait") \
    f(K_while,  "while")

#define CODE(t, str) + 1_s
constexpr auto Num_Keywords  = 0_s IMPALA_KEYWORDS(CODE);
#undef CODE

#define IMPALA_LIT(f) \
    f(L_s,        "<signed integer literal>") \
    f(L_u,        "<integer literal>") \
    f(L_f,        "<floating-point literal>") \

#define IMPALA_TOKENS(f) \
    /* misc */ \
    f(M_eof,          "<eof>") \
    f(M_id,           "<identifier>") \
    /* delimiters */ \
    f(D_angle_l,      "‹") \
    f(D_angle_r,      "›") \
    f(D_brace_l,      "{") \
    f(D_brace_r,      "}") \
    f(D_bracket_l,    "[") \
    f(D_bracket_r,    "]") \
    f(D_paren_l,      "(") \
    f(D_paren_r,      ")") \
    f(D_quote_l,      "«") \
    f(D_quote_r,      "»") \
    /* punctation */ \
    f(P_colon,        ":") \
    f(P_colon_colon,  "::") \
    f(P_comma,        ",") \
    f(P_dot,          ".") \
    f(P_semicolon,    ";") \
    /* operators */ \
    f(O_lambda,       "\\") \
    f(O_forall,       "\\/") \
    f(O_arrow,        "->") \
    f(O_inc,          "++") \
    f(O_dec,          "--") \
    f(O_eq,           "=") \
    f(O_add_eq,       "+=") \
    f(O_sub_eq,       "-=") \
    f(O_mul_eq,       "*=") \
    f(O_div_eq,       "/=") \
    f(O_mod_eq,       "%=") \
    f(O_shift_l_eq,   "<<=") \
    f(O_shift_r_eq,   ">>=") \
    f(O_and_eq,       "&=") \
    f(O_or_eq,        "|=") \
    f(O_xor_eq,       "^=") \
    f(O_add,          "+") \
    f(O_sub,          "-") \
    f(O_mul,          "*") \
    f(O_div,          "/") \
    f(O_mod,          "%") \
    f(O_tilde,        "~") \
    f(O_shift_l,      "<<") \
    f(O_shift_r,      ">>") \
    f(O_and,          "&") \
    f(O_and_and,      "&&") \
    f(O_or,           "|") \
    f(O_or_or,        "||") \
    f(O_xor,          "^") \
    f(O_not,          "!") \
    f(O_cmp_le,       "<=") \
    f(O_cmp_ge,       ">=") \
    f(O_cmp_lt,       "<") \
    f(O_cmp_gt,       ">") \
    f(O_cmp_eq,       "==") \
    f(O_cmp_ne,       "!=")

class Token {
public:
    enum class Tag {
#define CODE(t, str) t,
        IMPALA_KEYWORDS(CODE)
        IMPALA_LIT(CODE)
        IMPALA_TOKENS(CODE)
#undef CODE
    };

    enum class Prec {
        Bottom,
        Assign,
        Hlt,
        OrOr, AndAnd,
        Rel,
        Or, Xor, And,
        Shift, Add, Mul,
        Unary,
        RunRun,
        Error,
    };

    Token() {}
    Token(Location location, Tag tag)
        : location_(location)
        , tag_(tag)
        , symbol_(tag2str(tag))
    {}
    Token(Location location, thorin::s64 s)
        : location_(location)
        , tag_(Tag::L_s)
        , s64_(s)
    {}
    Token(Location location, thorin::u64 u)
        : location_(location)
        , tag_(Tag::L_u)
        , u64_(u)
    {}
    Token(Location location, thorin::f64 f)
        : location_(location)
        , tag_(Tag::L_f)
        , f64_(f)
    {}
    Token(Location location, Symbol symbol)
        : location_(location)
        , tag_(Tag::M_id)
        , symbol_(symbol)
    {}

    Tag tag() const { return tag_; }
    Location location() const { return location_; }
    //const std::string& identifier() const { assert(is_identifier()); return symbol_; }
    Symbol symbol() const { assert(tag() == Tag::M_id); return symbol_; }
    thorin::f64 f64() const { assert(tag() == Tag::L_f); return f64_; }
    thorin::s64 s64() const { assert(tag() == Tag::L_s); return s64_; }
    thorin::u64 u64() const { assert(tag() == Tag::L_u); return u64_; }
    bool is_literal() const {
        switch (tag()) {
#define CODE(t, str) case Tag::t: return true;
            IMPALA_LIT(CODE)
#undef CODE
            default: return false;
        }
    }

    bool isa(Tag tag) const { return tag_ == tag; }
    //bool operator == (const Token& token) const { return token.location_ == location_ && token.symbol_ == symbol_; }
    //bool operator != (const Token& token) const { return token.location_ != location_ || token.symbol_ != symbol_; }
    //uint32_t hash() const { return hash_combine(location_.hash(), hash_string(symbol_)); }

    static const char* tag2str(Tag tag) {
        switch (tag) {
#define CODE(t, str) case Tag::t: return str;
            IMPALA_KEYWORDS(CODE)
            IMPALA_LIT(CODE)
            IMPALA_TOKENS(CODE)
#undef CODE
            default: THORIN_UNREACHABLE;
        }
    }

    static Prec tag2prec(Tag tag) {
        switch (tag) {
            case Tag::O_eq:
            case Tag::O_add_eq:
            case Tag::O_sub_eq:
            case Tag::O_mul_eq:
            case Tag::O_div_eq:
            case Tag::O_mod_eq:
            case Tag::O_shift_l_eq:
            case Tag::O_shift_r_eq:
            case Tag::O_and_eq:
            case Tag::O_or_eq:
            case Tag::O_xor_eq:  return Prec::Assign;
            case Tag::O_or_or:   return Prec::OrOr;
            case Tag::O_and_and: return Prec::AndAnd;
            case Tag::O_not:
            case Tag::O_cmp_le:
            case Tag::O_cmp_ge:
            case Tag::O_cmp_lt:
            case Tag::O_cmp_gt:
            case Tag::O_cmp_eq:
            case Tag::O_cmp_ne:  return Prec::Rel;
            case Tag::O_or:      return Prec::Or;
            case Tag::O_xor:     return Prec::Xor;
            case Tag::O_and:     return Prec::And;
            case Tag::O_shift_l:
            case Tag::O_shift_r: return Prec::Shift;
            case Tag::O_add:
            case Tag::O_sub: return Prec::Add;
            case Tag::O_mul:
            case Tag::O_div:
            case Tag::O_mod: return Prec::Mul;
            default: return Prec::Error;
        }
    }

private:
    Location location_;
    Tag tag_;
    union {
        Symbol symbol_;
        thorin::f64 f64_;
        thorin::s64 s64_;
        thorin::u64 u64_;
    };
};

std::ostream& operator<<(std::ostream& os, const Token& t);

}

#endif

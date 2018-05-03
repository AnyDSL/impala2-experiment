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

#define IMPALA_OPS(f) \
    f(O_lambda,     "\\",  Error,   "") \
    f(O_forall,     "\\/", Error,   "") \
    f(O_arrow,      "->",  Error,   "") \
    f(O_inc,        "++",  Error,   "") \
    f(O_dec,        "--",  Error,   "") \
    f(O_assign,     "=",   Assign,  "") \
    f(O_add_assign, "+=",  Assign,  "add_assign") \
    f(O_sub_assign, "-=",  Assign,  "sub_assign") \
    f(O_mul_assign, "*=",  Assign,  "mul_assign") \
    f(O_div_assign, "/=",  Assign,  "div_assign") \
    f(O_rem_assign, "%=",  Assign,  "rem_assign") \
    f(O_shl_assign, "<<=", Assign,  "shl_assign") \
    f(O_shr_assign, ">>=", Assign,  "shr_assign") \
    f(O_and_assign, "&=",  Assign,  "bitand_assign") \
    f(O_or_assign,  "|=",  Assign,  "bitor_assign") \
    f(O_xor_assign, "^=",  Assign,  "bitxor_assign") \
    f(O_add,        "+",   Add,     "add") \
    f(O_sub,        "-",   Add,     "sub") \
    f(O_mul,        "*",   Mul,     "mul") \
    f(O_div,        "/",   Mul,     "div") \
    f(O_rem,        "%",   Mul,     "rem") \
    f(O_tilde,      "~",   Error,   "") \
    f(O_shl,        "<<",  Shift,   "shl") \
    f(O_shr,        ">>",  Shift,   "shr") \
    f(O_and,        "&",   And,     "bitand") \
    f(O_and_and,    "&&",  AndAnd,  "") \
    f(O_or,         "|",   Or,      "bitor") \
    f(O_or_or,      "||",  OrOr,    "") \
    f(O_xor,        "^",   Xor,     "bitxor") \
    f(O_not,        "!",   Error,   "") \
    f(O_le,         "<=",  Rel,     "le") \
    f(O_ge,         ">=",  Rel,     "ge") \
    f(O_lt,         "<",   Rel,     "lt") \
    f(O_gt,         ">",   Rel,     "gt") \
    f(O_eq,         "==",  Rel,     "eq") \
    f(O_ne,         "!=",  Rel,     "ne")

class Token {
public:
    enum class Tag {
#define CODE(t, str) t,
        IMPALA_KEYWORDS(CODE)
        IMPALA_LIT(CODE)
        IMPALA_TOKENS(CODE)
#undef CODE
#define CODE(t, str, prec, name) t,
        IMPALA_OPS(CODE)
#undef CODE
    };

    enum class Prec {
        Error,
        Bottom,
        Assign,
        Hlt,
        OrOr, AndAnd,
        Rel,
        Or, Xor, And,
        Shift, Add, Mul,
        Unary,
        RunRun,
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
    bool isa(Tag tag) const { return tag_ == tag; }
    Location location() const { return location_; }
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

    static const char* tag2str(Tag tag) {
        switch (tag) {
#define CODE(t, str) case Tag::t: return str;
            IMPALA_KEYWORDS(CODE)
            IMPALA_LIT(CODE)
            IMPALA_TOKENS(CODE)
#undef CODE
#define CODE(t, str, prec, name) case Tag::t: return str;
            IMPALA_OPS(CODE)
#undef CODE
            default: THORIN_UNREACHABLE;
        }
    }

    static Prec tag2prec(Tag tag) {
        switch (tag) {
#define CODE(t, str, prec, name) case Tag::t: return Prec::prec;
            IMPALA_OPS(CODE)
#undef CODE
            default: return Prec::Error;
        }
    }

    static const char* tag2name(Tag tag) {
        switch (tag) {
#define CODE(t, str, prec, name) case Tag::t: return name;
            IMPALA_OPS(CODE)
#undef CODE
            default: return "";
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

typedef Token::Tag TT;

}

#endif

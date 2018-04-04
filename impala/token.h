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
    f(K_cn,           "cn") \
    f(K_Cn,           "Cn") \
    f(K_else,         "else") \
    f(K_fn,           "fn") \
    f(K_Fn,           "Fn") \
    f(K_for,          "for") \
    f(K_if,           "if") \
    f(K_impl,         "impl") \
    f(K_let,          "let") \
    f(K_mut,          "mut") \
    f(K_self,         "Self") \
    f(K_struct,       "struct") \
    f(K_trait,        "trait")

#define IMPALA_TOKENS(f) \
    /* misc */ \
    f(M_eof,          "<eof>") \
    f(M_error,        "<unknown token>") \
    f(M_id,           "<identifier>") \
    f(M_lit,          "<literal>") \
    /* delimiters */ \
    f(D_l_brace,      "{") \
    f(D_r_brace,      "}") \
    f(D_l_bracket,    "[") \
    f(D_r_bracket,    "]") \
    f(D_l_paren,      "(") \
    f(D_r_paren,      ")") \
    /* punctation */ \
    f(P_dot,          ".") \
    f(P_dots,         "...") \
    f(P_comma,        ",") \
    f(P_semi,         ";") \
    f(P_colone_colon, "::") \
    f(P_colon,        ":") \
    /* operators */ \
    f(O_arrow,        "->") \
    f(O_inc,          "++") \
    f(O_dec,          "--") \
    f(O_eq,           "=") \
    f(O_add_eq,       "+=") \
    f(O_sub_eq,       "-=") \
    f(O_mul_eq,       "*=") \
    f(O_div_eq,       "/=") \
    f(O_mod_eq,       "%=") \
    f(O_l_shift_eq,   "<<=") \
    f(O_r_shift_eq,   ">>=") \
    f(O_and_eq,       "&=") \
    f(O_or_eq,        "|=") \
    f(O_xor_eq,       "^=") \
    f(O_add,          "+") \
    f(O_sub,          "-") \
    f(O_mul,          "*") \
    f(O_div,          "/") \
    f(O_mod,          "%") \
    f(O_l_shift,      "<<") \
    f(O_r_shift,      ">>") \
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
        IMPALA_TOKENS(CODE)
#undef CODE
    };

    Token(Location location)
        : Token(location, Tag::M_error)
    {}
    Token(Location location, Tag tag)
        : location_(location)
        , tag_(tag)
        , symbol_(tag_to_string(tag))
    {}
    Token(Location location, const char* str, Box box)
        : location_(location)
        , tag_(Tag::M_lit)
        , symbol_(str)
        , box_(box)
    {}
    Token(Location location, const char* str)
        : location_(location)
        , tag_(Tag::M_id)
        , symbol_(str)
    {}

    Tag tag() const { return tag_; }
    Location location() const { return location_; }
    Box box() const { return box_; }
    //const std::string& identifier() const { assert(is_identifier()); return symbol_; }
    Symbol symbol() const { return symbol_; }

    //bool is_identifier() const { return tag_ == M_id; }
    //bool is_literal() const { return tag_ == Lit; }


    //bool operator == (const Token& token) const { return token.location_ == location_ && token.symbol_ == symbol_; }
    //bool operator != (const Token& token) const { return token.location_ != location_ || token.symbol_ != symbol_; }
    //uint32_t hash() const { return hash_combine(location_.hash(), hash_string(symbol_)); }

    static const char* tag_to_string(Tag tag) {
        switch (tag) {
#define CODE(t, str) case Tag::t: return str;
            IMPALA_KEYWORDS(CODE)
            IMPALA_TOKENS(CODE)
#undef CODE
            default: THORIN_UNREACHABLE;
        }
    }

private:
    Location location_;
    Tag tag_;
    Symbol symbol_;
    Box box_;
};

std::ostream& operator<<(std::ostream& os, const Token& t);

}

#endif

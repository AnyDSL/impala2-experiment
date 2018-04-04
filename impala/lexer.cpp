#include "impala/lexer.h"

#include <stdexcept>

namespace impala {

// character classes
inline bool sp(uint32_t c)  { return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v'; }
inline bool dec(uint32_t c) { return c >= '0' && c <= '9'; }
inline bool hex(uint32_t c) { return dec(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
inline bool sym(uint32_t c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
inline bool bin(uint32_t c) { return '0' <= c && c <= '1'; }
inline bool oct(uint32_t c) { return '0' <= c && c <= '7'; }
inline bool eE(uint32_t c)  { return c == 'e' || c == 'E'; }
inline bool sgn(uint32_t c) { return c == '+' || c == '-'; }

Lexer::Lexer(Compiler& compiler, std::istream& is, const char* filename)
    : compiler(compiler)
    , stream_(is)
    , filename_(filename)
{
    if (!stream_)
        throw std::runtime_error("stream is bad");
    next();

    // eat utf-8 BOM if present
    accept(0xfeff, false);
}

inline bool is_bit_set(uint32_t val, uint32_t n) { return bool((val >> n) & 1_u32); }
inline bool is_bit_clear(uint32_t val, uint32_t n) { return !is_bit_set(val, n); }

// see https://en.wikipedia.org/wiki/UTF-8
uint32_t Lexer::next() {
    uint32_t result = peek_;
    uint32_t b1 = stream_.get();
    std::fill(peek_bytes_, peek_bytes_ + 4, 0);
    peek_bytes_[0] = b1;

    if (b1 == (uint32_t) std::istream::traits_type::eof()) {
        peek_ = b1;
        return result;
    }

    int n_bytes = 1;
    auto get_next_utf8_byte = [&] () {
        uint32_t b = stream_.get();
        peek_bytes_[n_bytes++] = b;
        if (is_bit_clear(b, 7) || is_bit_set(b, 6))
            error("invalid utf-8 character");
        return b & 0b00111111_u32;
    };

    auto update_peek = [&] (uint32_t peek) {
        back_line_ = peek_line_;
        back_col_  = peek_col_;
        ++peek_col_;
        peek_ = peek;
        return result;
    };

    if (is_bit_clear(b1, 7)) {
        // 1-byte: 0xxxxxxx
        back_line_ = peek_line_;
        back_col_  = peek_col_;

        if (b1 == '\n') {
            ++peek_line_;
            peek_col_ = 1;
        } else
            ++peek_col_;
        peek_ = b1;
        return result;
    } else {
        if (is_bit_set(b1, 6)) {
            if (is_bit_clear(b1, 5)) {
                // 2-bytes: 110xxxxx 10xxxxxx
                uint32_t b2 = get_next_utf8_byte();
                return update_peek((b1 & 0b00011111_u32) << 6_u32 | b2);
            } else if (is_bit_clear(b1, 4)) {
                // 3 bytes: 1110xxxx 10xxxxxx 10xxxxxx
                uint32_t b2 = get_next_utf8_byte();
                uint32_t b3 = get_next_utf8_byte();
                return update_peek((b1 & 0b00001111_u32) << 12_u32 | b2 << 6_u32 | b3);
            } else if (is_bit_clear(b1, 3)) {
                // 4 bytes: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                uint32_t b2 = get_next_utf8_byte();
                uint32_t b3 = get_next_utf8_byte();
                uint32_t b4 = get_next_utf8_byte();
                return update_peek((b1 & 0b00000111_u32) << 18_u32 | b2 << 12_u32 | b3 << 6_u32 | b4);
            }
        }
    }
    error("invalid utf-8 character");
    return 0;
}

void Lexer::eat_comments() {
    while (true) {
        while (!eof() && peek() != '*') next();
        if (eof()) {
            error("non-terminated multiline comment");
            return;
        }
        next();
        if (accept('/')) break;
    }
}

Token Lexer::lex() {
    while (true) {
        // skip whitespace
        if (accept_if(sp, false)) {
            while (accept_if(sp, false)) {}
            continue;
        }

        str_ = "";
        front_line_ = peek_line_;
        front_col_ = peek_col_;

        if (eof()) return {location(), Token::Tag::M_eof};

        if (accept('(')) return {location(), Token::Tag::D_l_paren};
        if (accept(')')) return {location(), Token::Tag::D_r_paren};
        if (accept('{')) return {location(), Token::Tag::D_l_brace};
        if (accept('}')) return {location(), Token::Tag::D_r_brace};
        if (accept('[')) return {location(), Token::Tag::D_l_bracket};
        if (accept(']')) return {location(), Token::Tag::D_r_bracket};

        if (accept('.')) {
            if (accept('.')) {
                if (accept('.')) return {location(), Token::Tag::P_dots};
                error("unknown token '..'");
                return {location()};
            }
            return {location(), Token::Tag::P_dot};
        }
        if (accept(',')) return {location(), Token::Tag::P_comma};
        if (accept(';')) return {location(), Token::Tag::P_semi};
        if (accept(':')) {
            if (accept(':')) return {location(), Token::Tag::P_colone_colon};
            return {location(), Token::Tag::P_colon};
        }
        if (accept('=')) {
            if (accept('=')) return {location(), Token::Tag::O_cmp_eq};
            return {location(), Token::Tag::O_eq};
        }
        if (accept('<')) {
            if (accept('<')) {
                if (accept('=')) return {location(), Token::Tag::O_l_shift_eq};
                return {location(), Token::Tag::O_l_shift};
            }
            if (accept('=')) return {location(), Token::Tag::O_cmp_le};
            return {location(), Token::Tag::O_cmp_lt};
        }
        if (accept('>')) {
            if (accept('>')) {
                if (accept('=')) return {location(), Token::Tag::O_r_shift_eq};
                return {location(), Token::Tag::O_r_shift};
            }
            if (accept('=')) return {location(), Token::Tag::O_cmp_ge};
            return {location(), Token::Tag::O_cmp_gt};
        }
        if (accept('+')) {
            if (accept('+')) return {location(), Token::Tag::O_inc};
            if (accept('=')) return {location(), Token::Tag::O_add_eq};
            return {location(), Token::Tag::O_add};
        }
        if (accept('-')) {
            if (accept('>')) return {location(), Token::Tag::O_arrow};
            if (accept('-')) return {location(), Token::Tag::O_dec};
            if (accept('=')) return {location(), Token::Tag::O_sub_eq};
            return {location(), Token::Tag::O_sub};
        }
        if (accept('*')) {
            if (accept('=')) return {location(), Token::Tag::O_mul_eq};
            return {location(), Token::Tag::O_mul};
        }
        if (accept('/')) {
            // Handle comments here
            if (accept('*')) { eat_comments(); continue; }
            if (accept('/')) {
                while (!eof() && peek() != '\n') next();
                continue;
            }
            if (accept('=')) return {location(), Token::Tag::O_div_eq};
            return {location(), Token::Tag::O_div};
        }
        if (accept('%')) {
            if (accept('=')) return {location(), Token::Tag::O_mod_eq};
            return {location(), Token::Tag::O_mod};
        }
        if (accept('&')) {
            if (accept('&')) return {location(), Token::Tag::O_and_and};
            if (accept('=')) return {location(), Token::Tag::O_and_eq};
            return {location(), Token::Tag::O_and};
        }
        if (accept('|')) {
            if (accept('|')) return {location(), Token::Tag::O_or_or};
            if (accept('=')) return {location(), Token::Tag::O_or_eq};
            return {location(), Token::Tag::O_or};
        }
        if (accept('^')) {
            if (accept('=')) return {location(), Token::Tag::O_xor_eq};
            return {location(), Token::Tag::O_xor};
        }

        if (accept('!')) {
            if (accept('=')) return {location(), Token::Tag::O_cmp_ne};
            return {location(), Token::Tag::O_not};
        }

        if (std::isdigit(peek()) || peek() == '.') {
            auto lit = parse_literal();
            return {location(), str_.c_str(), lit};
        }

        if (std::isalpha(peek()) || peek() == '_') {
            accept();
            while (std::isalnum(peek()) || peek() == '_') accept();

            if (str_ == "true")  return Token(loc_, str_, true);
            if (str_ == "false") return Token(loc_, str_, false);

            auto key_it = keywords.find(str_);
            if (key_it == keywords.end()) return Token(loc_, str_);
            return Token(loc_, key_it->second);
        }

        // TODO utf-8 stuff here

        if (dec(peek()) || sgn(peek())) {
            auto lit = parse_literal();
            return {location(), lit};
        }

#if 0
        // identifier
        if (accept_if(sym)) {
            while (accept_if(sym) || accept_if(dec)) {}

            // TODO make this mechanism better
            if (str() == "cn")   return {location(), Token::Tag::Tag::Cn};
            if (str() == "bool") return {location(), Token::Tag::Tag::Bool};
            return {location(), str()};
        }
#endif

        error("invalid character '{}'", peek_bytes_);
        next();
    }
}

Literal Lexer::parse_literal() {
    int base = 10;

    auto parse_digits = [&] () {
        switch (base) {
            case  2: while (accept_if(bin)) {} break;
            case  8: while (accept_if(oct)) {} break;
            case 10: while (accept_if(dec)) {} break;
            case 16: while (accept_if(hex)) {} break;
        }
    };

    // sign
    bool sign = false;
    if (accept('+')) {}
    else if (accept('-')) { sign = true; }

    // prefix starting with '0'
    if (accept('0', false)) {
        if      (accept('b', false)) base = 2;
        else if (accept('x', false)) base = 16;
        else if (accept('o', false)) base = 8;
    }

    parse_digits();

    bool exp = false, fract = false;
    if (base == 10) {
        // parse fractional part
        if (accept('.')) {
            fract = true;
            parse_digits();
        }

        // parse exponent
        if (accept_if(eE)) {
            exp = true;
            if (accept_if(sgn)) {}
            parse_digits();
        }
    }

    // suffix
    if (!exp && !fract) {
        if (accept('s', false)) {
            if (accept("8",  false)) return {Literal::Tag::Lit_s8,   s8( strtol(str().c_str(), nullptr, base))};
            if (accept("16", false)) return {Literal::Tag::Lit_s16, s16( strtol(str().c_str(), nullptr, base))};
            if (accept("32", false)) return {Literal::Tag::Lit_s32, s32( strtol(str().c_str(), nullptr, base))};
            if (accept("64", false)) return {Literal::Tag::Lit_s64, s64(strtoll(str().c_str(), nullptr, base))};
        }

        if (!sign) {
            if (accept('u', false)) {
                if (accept("8", false))  return {Literal::Tag::Lit_u8,   u8( strtoul(str().c_str(), nullptr, base))};
                if (accept("16", false)) return {Literal::Tag::Lit_u16, u16( strtoul(str().c_str(), nullptr, base))};
                if (accept("32", false)) return {Literal::Tag::Lit_u32, u32( strtoul(str().c_str(), nullptr, base))};
                if (accept("64", false)) return {Literal::Tag::Lit_u64, u64(strtoull(str().c_str(), nullptr, base))};
            }

            if (accept(0x002090))
                return {Literal::Tag::Lit_arity, u64(strtoull(str().c_str(), nullptr, base))};

            if (peek() > 0x002080 && peek() < 0x002090) {
                return {Literal::Tag::Lit_index, u64(strtoull(str().c_str(), nullptr, base))};
            }
        }
    }

    if (base == 10 && accept('r', false)) {
        if (accept("16", false)) return {Literal::Tag::Lit_r16, r16(strtof(str().c_str(), nullptr))};
        if (accept("32", false)) return {Literal::Tag::Lit_r32, r32(strtof(str().c_str(), nullptr))};
        if (accept("64", false)) return {Literal::Tag::Lit_r64, r64(strtod(str().c_str(), nullptr))};
    }

    // untyped literals
    if (base == 10 && !fract && !exp) {
        return Literal(Literal::Tag::Lit_untyped, u64(strtoull(str().c_str(), nullptr, 10)));
    }

    error("invalid literal");
}

}

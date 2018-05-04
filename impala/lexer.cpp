#include "impala/lexer.h"

#include <stdexcept>

namespace impala {

// character classes
inline bool wsp(uint32_t c) { return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v'; }
inline bool dec(uint32_t c) { return c >= '0' && c <= '9'; }
inline bool hex(uint32_t c) { return dec(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
inline bool sym(uint32_t c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
inline bool bin(uint32_t c) { return '0' <= c && c <= '1'; }
inline bool oct(uint32_t c) { return '0' <= c && c <= '7'; }
inline bool eE (uint32_t c) { return c == 'e' || c == 'E'; }
inline bool sgn(uint32_t c) { return c == '+' || c == '-'; }

Lexer::Lexer(Compiler& compiler, std::istream& is, const char* filename)
    : compiler(compiler)
    , stream_(is)
    , filename_(filename)
{
    size_t i = 0;
#define CODE(tag, str) keywords_[i++] = {Symbol(str), TT::tag};
    IMPALA_KEYWORDS(CODE)
#undef CODE

    if (!stream_) throw std::runtime_error("stream is bad");
    next();
    accept(0xfeff, false); // eat utf-8 BOM if present
    front_line_ = front_col_  = 1;
    back_line_  = back_col_   = 1;
    peek_line_  = peek_col_   = 1;
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
        back_line_ = peek_line_;
        back_col_  = peek_col_;
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
            peek_col_ = 0;
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
        str_ = "";
        front_line_ = peek_line_;
        front_col_  = peek_col_;

        // end of file
        if (eof()) return {loc(), TT::M_eof};

        // skip whitespace
        if (accept_if(wsp, false)) {
            while (accept_if(wsp, false)) {}
            continue;
        }

        // delimiters
        if (accept( '(')) return {loc(), TT::D_paren_l};
        if (accept( ')')) return {loc(), TT::D_paren_r};
        if (accept( '[')) return {loc(), TT::D_bracket_l};
        if (accept( ']')) return {loc(), TT::D_bracket_r};
        if (accept( '{')) return {loc(), TT::D_brace_l};
        if (accept( '}')) return {loc(), TT::D_brace_r};
        if (accept(U'«')) return {loc(), TT::D_quote_l};
        if (accept(U'»')) return {loc(), TT::D_quote_r};
        if (accept(U'‹')) return {loc(), TT::D_angle_l};
        if (accept(U'›')) return {loc(), TT::D_angle_r};

        // punctation
        if (accept('.')) return {loc(), TT::P_dot};
        if (accept(',')) return {loc(), TT::P_comma};
        if (accept(';')) return {loc(), TT::P_semicolon};
        if (accept(':')) {
            if (accept(':')) return {loc(), TT::P_colon_colon};
            return {loc(), TT::P_colon};
        }

        // operators
        if (accept('\\')) {
            if (accept('/')) return {loc(), TT::O_forall};
            return {loc(), TT::O_lambda};
        }
        if (accept('=')) {
            if (accept('=')) return {loc(), TT::O_eq};
            return {loc(), TT::O_assign};
        }
        if (accept('<')) {
            if (accept('<')) {
                if (accept('=')) return {loc(), TT::O_shl_assign};
                return {loc(), TT::O_shl};
            }
            if (accept('=')) return {loc(), TT::O_le};
            return {loc(), TT::O_lt};
        }
        if (accept('>')) {
            if (accept('>')) {
                if (accept('=')) return {loc(), TT::O_shr_assign};
                return {loc(), TT::O_shr};
            }
            if (accept('=')) return {loc(), TT::O_ge};
            return {loc(), TT::O_gt};
        }
        if (accept('+')) {
            if (accept('+')) return {loc(), TT::O_inc};
            if (accept('=')) return {loc(), TT::O_add_assign};
            return {loc(), TT::O_add};
        }
        if (accept('-')) {
            if (accept('>')) return {loc(), TT::O_arrow};
            if (accept('-')) return {loc(), TT::O_dec};
            if (accept('=')) return {loc(), TT::O_sub_assign};
            return {loc(), TT::O_sub};
        }
        if (accept('*')) {
            if (accept('=')) return {loc(), TT::O_mul_assign};
            return {loc(), TT::O_mul};
        }
        if (accept('/')) {
            // Handle comments here
            if (accept('*')) { eat_comments(); continue; }
            if (accept('/')) {
                while (!eof() && peek() != '\n') next();
                continue;
            }
            if (accept('='))  return {loc(), TT::O_div_assign};
            return {loc(), TT::O_div};
        }
        if (accept('%')) {
            if (accept('=')) return {loc(), TT::O_rem_assign};
            return {loc(), TT::O_rem};
        }
        if (accept('&')) {
            if (accept('&')) return {loc(), TT::O_and_and};
            if (accept('=')) return {loc(), TT::O_and_assign};
            return {loc(), TT::O_and};
        }
        if (accept('|')) {
            if (accept('|')) return {loc(), TT::O_or_or};
            if (accept('=')) return {loc(), TT::O_or_assign};
            return {loc(), TT::O_or};
        }
        if (accept('^')) {
            if (accept('=')) return {loc(), TT::O_xor_assign};
            return {loc(), TT::O_xor};
        }
        if (accept('!')) {
            if (accept('=')) return {loc(), TT::O_ne};
            return {loc(), TT::O_not};
        }
        if (dec(peek()) || sgn(peek()))
            return parse_literal();

        // identifier
        if (accept_if(sym)) {
            while (accept_if(sym) || accept_if(dec)) {}
            Symbol symbol(str_);
            auto i = std::find_if(keywords_.begin(), keywords_.end(), [&](auto p) { return p.first == symbol; });
            return i == keywords_.end() ? Token{loc(), symbol} : Token{loc(), i->second};
        }

        // TODO utf-8 stuff here

        error("invalid character '{}'", peek_bytes_);
        next();
    }
}

Token Lexer::parse_literal() {
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

    bool is_float = false;
    if (base == 10) {
        // parse fractional part
        if (accept('.')) {
            is_float = true;
            parse_digits();
        }

        // parse exponent
        if (accept_if(eE)) {
            is_float = true;
            if (accept_if(sgn)) {}
            parse_digits();
        }
    }

    if (is_float)
        return {loc(), thorin::s64(strtoll(str().c_str(), nullptr, base))};
    if (sign)
        return {loc(), thorin::u64(strtoll(str().c_str(), nullptr, base))};
    return {loc(), thorin::u64(strtoull(str().c_str(), nullptr, base))};
}

}

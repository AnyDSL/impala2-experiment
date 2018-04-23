#include "impala/token.h"

namespace impala {

std::ostream& operator<<(std::ostream& os, const Token& token) {
    //os << token.location() << ": ";
    switch (token.tag()) {
        case Token::Tag::M_id: return os << token.symbol();
        case Token::Tag::L_s:  return os << token.s64();
        case Token::Tag::L_u:  return os << token.u64();
        case Token::Tag::L_f:  return os << token.f64();
        default: return os << Token::tag2string(token.tag());
    }
    THORIN_UNREACHABLE;
}

}

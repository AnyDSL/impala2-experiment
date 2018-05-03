#include "impala/token.h"

namespace impala {

std::ostream& operator<<(std::ostream& os, const Token& token) {
    //os << token.location() << ": ";
    switch (token.tag()) {
        case TT::M_id: return os << token.symbol();
        case TT::L_s:  return os << token.s64();
        case TT::L_u:  return os << token.u64();
        case TT::L_f:  return os << token.f64();
        default: return os << Token::tag2str(token.tag());
    }
    THORIN_UNREACHABLE;
}

}

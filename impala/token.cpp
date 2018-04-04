#include "impala/token.h"

namespace impala {

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.location() << ": ";
    switch (token.tag()) {
        case Token::Tag::M_id:  [[fallthrough]];
        case Token::Tag::M_lit: return os << token.symbol();
        default: return os << Token::tag_to_string(token.tag());
    }
    THORIN_UNREACHABLE;
}

}

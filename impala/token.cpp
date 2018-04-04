#include "impala/token.h"

namespace impala {

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << token.location() << ": ";
    if (tag() == Token::Tag::Identifier)
        return os << token.symbol();
    if (token.isa(Token::Tag::Literal))
        return os << token.literal().box.get_u64();
    return os << Token::tag_to_string(token.tag());
}

}

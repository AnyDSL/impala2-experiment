#include "impala/print.h"

#include "impala/ast.h"

namespace impala {

std::ostream& Node::stream(std::ostream& os) const {
    Printer printer(os);
    print(printer);
    return os;
}

void Node::print(std::ostream& os, bool fancy) const {
    Printer printer(os, fancy);
    print(printer);
}

//------------------------------------------------------------------------------

void Node::print(Printer&) const {
    // TODO
}

}

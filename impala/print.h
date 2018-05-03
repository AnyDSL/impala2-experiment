#ifndef IMPALA_PRINT_H
#define IMPALA_PRINT_H

#include "thorin/util/stream.h"

namespace impala {

class Printer : public thorin::IndentPrinter<Printer> {
public:
    explicit Printer(std::ostream& ostream, bool fancy = false, const char* tab = "    ")
        : thorin::IndentPrinter<Printer>(ostream, tab)
        , fancy_(fancy)
    {}

    bool fancy() const { return fancy_; }

private:
    bool fancy_;
};

}

namespace thorin {
template void Streamable<impala::Printer>::dump() const;
}

#endif


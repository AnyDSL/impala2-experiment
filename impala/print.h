#ifndef IMPALA_PRINT_H
#define IMPALA_PRINT_H

#include <ostream>

namespace impala {

class Printer {
public:
    explicit Printer(std::ostream& ostream, bool fancy = false)
        : ostream_(ostream)
        , fancy_(fancy)
    {}

    std::ostream& ostream() { return ostream_; };

private:
    std::ostream& ostream_;
    bool fancy_;
};

}

#endif


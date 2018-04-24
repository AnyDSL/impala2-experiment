#ifndef IMPALA_PRINT_H
#define IMPALA_PRINT_H

#include <cassert>
#include <ostream>

namespace impala {

class Printer {
public:
    explicit Printer(std::ostream& ostream, bool fancy = false)
        : ostream_(ostream)
        , fancy_(fancy)
    {}

    std::ostream& ostream() { return ostream_; };
    bool fancy() const { return fancy_; }
    Printer& indent() { ++level_; return *this; }
    Printer& dedent() { assert(level_ > 0); --level_; return *this; }
    Printer& endl();
    template<class T> Printer& operator<<(const T& val) { ostream() << val; return *this; }

private:
    std::ostream& ostream_;
    int level_ = 0;
    bool fancy_;
};

}

#endif


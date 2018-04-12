#ifndef IMPALA_COMPILER_H
#define IMPALA_COMPILER_H

#include "impala/sema/world.h"

namespace impala {

using thorin::Location;

class Compiler {
public:
    Compiler(const Compiler&) = delete;
    Compiler(Compiler&&) = delete;
    Compiler& operator=(Compiler) = delete;
    Compiler() = default;

    int num_warnings() const { return num_warnings_; }
    int num_errors() const { return num_errors_; }

    template<typename... Args>
    std::ostream& warning(Location location, const char* fmt, Args... args) {
        ++num_warnings_;
        thorin::streamf(std::cerr, "{}: warning: ", location);
        return thorin::streamf(std::cerr, fmt, args...) << std::endl;;
    }
    template<typename... Args>
    std::ostream& error(Location location, const char* fmt, Args... args) {
        ++num_errors_;
        thorin::streamf(std::cerr, "{}: error: ", location);
        return thorin::streamf(std::cerr, fmt, args...) << std::endl;;
    }

    World world;

private:
    int num_warnings_ = 0;
    int num_errors_ = 0;
};

}

#endif

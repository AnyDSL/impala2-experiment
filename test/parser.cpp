#include "gtest/gtest.h"

#include <sstream>
#include <string>

#include "thorin/util/stream.h"
#include "impala/parser.h"

using namespace impala;

TEST(Parser, PackOrTuple) {
    Compiler compiler;
    auto a = parse("()");
    auto b = parse("(x: int; y)");
    auto c = parse("(int; y)");
    auto d = parse("(x: int, y)");
    auto e = parse("(int, y)");
}

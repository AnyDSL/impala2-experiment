#include "gtest/gtest.h"

#include <sstream>
#include <string>

#include "thorin/util/stream.h"
#include "impala/parser.h"

using namespace impala;

TEST(Parser, PackOrTuple) {
    Compiler compiler;
    parse(compiler, "()");

    parse(compiler, "(x: int; y)");
    parse(compiler, "(int; y)");

    parse(compiler, "(x,      y)");
    parse(compiler, "(x: int, y)");
    parse(compiler, "(x: int, y: int)");
    parse(compiler, "(x,      y: int)");

    parse(compiler, "(x,      y,)");
    parse(compiler, "(x: int, y,)");
    parse(compiler, "(x: int, y: int,)");
    parse(compiler, "(x,      y: int,)");

    parse(compiler, "(x,      y): T");
    parse(compiler, "(x: int, y): T");
    parse(compiler, "(x: int, y: int): T");
    parse(compiler, "(x,      y: int): T");

    parse(compiler, "(x,      y,): T");
    parse(compiler, "(x: int, y,): T");
    parse(compiler, "(x: int, y: int,): T");
    parse(compiler, "(x,      y: int,): T");
    EXPECT_EQ(compiler.num_errors(), 0);
}

TEST(Parser, SigmaOrVariadic) {
    Compiler compiler;
    parse(compiler, "[]");

    parse(compiler, "[x: int; y]");
    parse(compiler, "[int; y]");

    parse(compiler, "[x,      y]");
    parse(compiler, "[x: int, y]");
    parse(compiler, "[x: int, y: int]");
    parse(compiler, "[x,      y: int]");
    parse(compiler, "[x,      y,]");
    parse(compiler, "[x: int, y,]");
    parse(compiler, "[x: int, y: int,]");
    parse(compiler, "[x,      y: int,]");
    EXPECT_EQ(compiler.num_errors(), 0);
}

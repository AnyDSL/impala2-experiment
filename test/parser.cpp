#include "gtest/gtest.h"

#include <sstream>
#include <string>

#include "thorin/util/stream.h"
#include "impala/parser.h"

using namespace impala;

TEST(Parser, Pack) {
    Compiler compiler;
    parse(compiler, "pk(x: int; y)");
    parse(compiler, "pk(int; y)");
    EXPECT_EQ(compiler.num_errors(), 0);
}

TEST(Parser, Tuple) {
    Compiler compiler;

    parse(compiler, "()");
    parse(compiler, "(x,   y)");
    parse(compiler, "(x=a, y)");
    parse(compiler, "(x=a, y=b)");
    parse(compiler, "(x,   y=b)");

    parse(compiler, "(x,   y,)");
    parse(compiler, "(x=a, y,)");
    parse(compiler, "(x=a, y=b,)");
    parse(compiler, "(x,   y=b,)");

    parse(compiler, "(x,   y): T");
    parse(compiler, "(x=a, y): T");
    parse(compiler, "(x=a, y=b): T");
    parse(compiler, "(x,   y=b): T");

    parse(compiler, "(x,   y,): T");
    parse(compiler, "(x=a, y,): T");
    parse(compiler, "(x=a, y=b,): T");
    parse(compiler, "(x,   y=b,): T");
    EXPECT_EQ(compiler.num_errors(), 0);
}

TEST(Parser, Sigma) {
    Compiler compiler;
    parse(compiler, "[]");
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

TEST(Parser, Variadic) {
    Compiler compiler;
    parse(compiler, "ar[x: int; y]");
    parse(compiler, "ar[int; y]");
    EXPECT_EQ(compiler.num_errors(), 0);
}

TEST(Parser, Stmnts) {
    Compiler compiler;
    parse(compiler, "if cond { x }");
    parse(compiler, "if cond { x } else { y }");
    parse(compiler, "if cond { x } else if { y }");
    parse(compiler, "if cond { x } else if { y } else { z }");

    parse(compiler, "{ foo; if cond { x } }");
    parse(compiler, "{ foo; if cond { x } else { y } }");
    parse(compiler, "{ foo; if cond { x } else if cond { y } }");
    parse(compiler, "{ foo; if cond { x } else if cond { y } else { z } }");

    parse(compiler, "{ if cond { x } foo }");
    parse(compiler, "{ if cond { x } else { y } foo }");
    parse(compiler, "{ if cond { x } else if cond { y } foo }");
    parse(compiler, "{ if cond { x } else if cond { y } else { z } foo }");

    parse(compiler, "{ if cond { x }; foo }");
    parse(compiler, "{ if cond { x } else { y }; foo }");
    parse(compiler, "{ if cond { x } else if cond { y }; foo }");
    parse(compiler, "{ if cond { x } else if cond { y } else { z }; foo }");

    EXPECT_EQ(compiler.num_errors(), 0);
}

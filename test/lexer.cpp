#include "gtest/gtest.h"

#include <sstream>
#include <string>

#include "thorin/util/stream.h"
#include "impala/lexer.h"

using namespace impala;

TEST(Lexer, Tokens) {
    Compiler compiler;
    std::istringstream is("{ } ( ) [ ] ‹ › « » : , . \\ \\/ \\u ᵁ \\r ᴿ \\a ᴬ \\l ᴸ");
    Lexer lexer(compiler, is, "stdin");

    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_brace_l));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_brace_r));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_paren_l));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_paren_r));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_bracket_l));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_bracket_r));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_angle_l));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_angle_r));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_quote_l));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_quote_r));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::P_colon));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::P_comma));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::P_dot));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::B_lambda));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::B_forall));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_u));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_u));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_r));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_r));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_a));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_a));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_l));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::U_l));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::M_eof));
}

TEST(Lexer, LocId) {
    Compiler compiler;
    std::istringstream is(" test  abc    def if  \nwhile   foo   ");
    Lexer lexer(compiler, is, "stdin");
    auto t1 = lexer.lex();
    auto t2 = lexer.lex();
    auto t3 = lexer.lex();
    auto t4 = lexer.lex();
    auto t5 = lexer.lex();
    auto t6 = lexer.lex();
    auto t7 = lexer.lex();
    std::ostringstream oss;
    thorin::streamf(oss, "{} {} {} {} {} {} {}", t1, t2, t3, t4, t5, t6, t7);
    EXPECT_EQ(oss.str(), "test abc def if while foo <eof>");
    EXPECT_EQ(t1.loc(), Loc("stdin", 1,  2, 1,  5));
    EXPECT_EQ(t2.loc(), Loc("stdin", 1,  8, 1, 10));
    EXPECT_EQ(t3.loc(), Loc("stdin", 1, 15, 1, 17));
    EXPECT_EQ(t4.loc(), Loc("stdin", 1, 19, 1, 20));
    EXPECT_EQ(t5.loc(), Loc("stdin", 2,  1, 2,  5));
    EXPECT_EQ(t6.loc(), Loc("stdin", 2,  9, 2, 11));
    EXPECT_EQ(t7.loc(), Loc("stdin", 2, 14, 2, 14));
}

TEST(Lexer, Literals) {
}

TEST(Lexer, Utf8) {
}

TEST(Lexer, Eof) {
    Compiler compiler;
    std::istringstream is("");

    Lexer lexer(compiler, is, "stdin");
    for (int i = 0; i < 100; i++)
        EXPECT_TRUE(lexer.lex().isa(Token::Tag::M_eof));
}

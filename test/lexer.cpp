#include "gtest/gtest.h"

#include <sstream>
#include <string>

#include "thorin/util/log.h"
#include "impala/lexer.h"
#include "impala/lexer.h"

using namespace impala;

TEST(Lexer, Tokens) {
    Compiler compiler;
    std::istringstream is("{ } ( ) [ ] : , .");
    Lexer lexer(compiler, is, "stdin");

    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_l_brace));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_r_brace));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_l_paren));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_r_paren));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_l_bracket));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::D_r_bracket));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::P_colon));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::P_comma));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::P_dot));
    EXPECT_TRUE(lexer.lex().isa(Token::Tag::M_eof));
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

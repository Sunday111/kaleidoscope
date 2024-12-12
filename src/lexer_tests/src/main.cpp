#include "gtest/gtest.h"
#include "kaleidoscope/lexer/lexer.hpp"

TEST(LexerTest, IntTokens)
{
    kaleidoscope::Lexer lexer("0");
    using namespace kaleidoscope;
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_TRUE(token->type == TokenType::IntegralLiteral);
}

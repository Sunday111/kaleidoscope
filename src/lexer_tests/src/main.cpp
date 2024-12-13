#include "gtest/gtest.h"
#include "kaleidoscope/lexer/lexer.hpp"

[[nodiscard]] constexpr kaleidoscope::LexerToken EofToken(size_t text_len)
{
    return kaleidoscope::LexerToken{
        .type = kaleidoscope::TokenType::EndOfFile,
        .begin = text_len,
        .end = text_len,
    };
}

TEST(LexerTest, ZeroInt)
{
    kaleidoscope::Lexer lexer("0");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::DecimalLiteral,
            .begin = 0,
            .end = 1,
        }));
}

TEST(LexerTest, TwoZeroInts)
{
    kaleidoscope::Lexer lexer("0 0");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token,
        (LexerToken{
            .type = TokenType::DecimalLiteral,
            .begin = 0,
            .end = 1,
        }));

    token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::DecimalLiteral,
            .begin = 2,
            .end = 3,
        }));

    token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(token, EofToken(3));
}

TEST(LexerTest, TwoLeadingZeroes)
{
    kaleidoscope::Lexer lexer("001");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::OctalLiteral,
            .begin = 0,
            .end = 3,
        }));
}

TEST(LexerTest, ZeroAndDot)
{
    kaleidoscope::Lexer lexer("0.");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::FloatLiteral,
            .begin = 0,
            .end = 2,
        }));
}

TEST(LexerTest, JustDot)
{
    kaleidoscope::Lexer lexer(".");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_FALSE(token.has_value());
    ASSERT_EQ(
        token.error(),
        (LexerError{
            .type = LexerErrorType::NeedAtLeastOneDigitAroundDotInFloatLiteral,
            .pos = 0,
        }));
}

TEST(LexerTest, ExpAfterDot)
{
    kaleidoscope::Lexer lexer("0.e3");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::FloatLiteral,
            .begin = 0,
            .end = 4,
        }));
}

TEST(LexerTest, ZeroLengthSignificand)
{
    kaleidoscope::Lexer lexer(".e3");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_FALSE(token.has_value());
    ASSERT_EQ(
        token.error(),
        (LexerError{
            .type = LexerErrorType::NeedAtLeastOneDigitAroundDotInFloatLiteral,
            .pos = 0,
        }));
}

TEST(LexerTest, HexValue0x0)
{
    kaleidoscope::Lexer lexer("0x0");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::HexadecimalLiteral,
            .begin = 0,
            .end = 3,
        }));
}

TEST(LexerTest, HexValue0X0)
{
    kaleidoscope::Lexer lexer("0X0");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::HexadecimalLiteral,
            .begin = 0,
            .end = 3,
        }));
}

TEST(LexerTest, HexValue0xABCDEF0123456789)
{
    kaleidoscope::Lexer lexer("0xABCDEF0123456789");
    using namespace kaleidoscope;  // NOLINT
    auto token = lexer.GetToken();
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(
        token.value(),
        (LexerToken{
            .type = TokenType::HexadecimalLiteral,
            .begin = 0,
            .end = 18,
        }));
}

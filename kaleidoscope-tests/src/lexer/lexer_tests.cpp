#include "util.hpp"

TEST(LexerUtilityTests, BitsetFromChars)
{
    auto bitset = BitsetFromChars("abc");
    ASSERT_TRUE(bitset.Set(std::bit_cast<uint8_t>('a'), false));
    ASSERT_TRUE(bitset.Set(std::bit_cast<uint8_t>('b'), false));
    ASSERT_TRUE(bitset.Set(std::bit_cast<uint8_t>('c'), false));
    ASSERT_EQ(bitset.CountOnes(), 0uz);
}

TEST(LexerUtilityTests, BitsetFromCharRange)
{
    auto bitset = BitsetFromCharRange('a', 'c');
    ASSERT_TRUE(bitset.Set(std::bit_cast<uint8_t>('a'), false));
    ASSERT_TRUE(bitset.Set(std::bit_cast<uint8_t>('b'), false));
    ASSERT_TRUE(bitset.Set(std::bit_cast<uint8_t>('c'), false));
    ASSERT_EQ(bitset.CountOnes(), 0uz);
}

TEST(LexerTest, DecimalLiterals)
{
    CheckLexerOutput(
        std::source_location::current(),
        "0 11",
        {
            Tok("0", kDecimalLiteral),
            Tok("11", kDecimalLiteral),
            Tok("", kEOF),
        });
}

TEST(LexerTest, FloatLiterals)
{
    CheckLexerOutput(
        std::source_location::current(),
        ". 0. .0 ☺ 0..",
        {
            Err(".", LexerErrorType::NeedAtLeastOneDigitAroundDotInFloatLiteral),
            Tok("0.", kFloatLiteral),
            Tok(".0", kFloatLiteral),
            Err("☺", LexerErrorType::UnexpectedSymbol),
            Err("0..", LexerErrorType::MultipleDotsInFloatingPointLiteral),
            Tok("", kEOF),
        });
}

TEST(LexerTest, OctalLiterals)
{
    CheckLexerOutput(
        std::source_location::current(),
        "00 01 001A",
        {
            Tok("00", kOctalLiteral),
            Tok("01", kOctalLiteral),
            Err("001A", LexerErrorType::UnexpectedSymbol),
            Tok("", kEOF),
        });
}

TEST(LexerTest, ScientificNotationLiterals)
{
    CheckLexerOutput(
        std::source_location::current(),
        "1e3 1e-3 2e+3 0.e3 0.e0 .e3 .0 0.e 0.e+ 0.e- 1E3 1E-3 2E+3 0.E3 0.E0 .E3 0.E 0.E+ 0.E- 0.E-1a",
        {
            Tok("1e3", kFloatLiteral),
            Tok("1e-3", kFloatLiteral),
            Tok("2e+3", kFloatLiteral),
            Tok("0.e3", kFloatLiteral),
            Tok("0.e0", kFloatLiteral),
            Err(".e3", LexerErrorType::NeedAtLeastOneDigitAroundDotInFloatLiteral),
            Tok(".0", kFloatLiteral),
            Err("0.e", LexerErrorType::ZeroLengthExponentInScientificNotation),
            Err("0.e+", LexerErrorType::ZeroLengthExponentInScientificNotation),
            Err("0.e-", LexerErrorType::ZeroLengthExponentInScientificNotation),
            Tok("1E3", kFloatLiteral),
            Tok("1E-3", kFloatLiteral),
            Tok("2E+3", kFloatLiteral),
            Tok("0.E3", kFloatLiteral),
            Tok("0.E0", kFloatLiteral),
            Err(".E3", LexerErrorType::NeedAtLeastOneDigitAroundDotInFloatLiteral),
            Err("0.E", LexerErrorType::ZeroLengthExponentInScientificNotation),
            Err("0.E+", LexerErrorType::ZeroLengthExponentInScientificNotation),
            Err("0.E-", LexerErrorType::ZeroLengthExponentInScientificNotation),
            Err("0.E-1a", LexerErrorType::UnexpectedSymbol),
            Tok("", kEOF),
        });
}

TEST(LexerTest, HexLiterals)
{
    CheckLexerOutput(
        std::source_location::current(),
        "0x0 0X0 0x 0x8a 0X8A 0xF 0XF 0xG 0XZ 0x1z 0xABCDEF0123456789",
        {
            Tok("0x0", kHexLiteral),
            Tok("0X0", kHexLiteral),
            Err("0x", LexerErrorType::UnexpectedSymbol),
            Tok("0x8a", kHexLiteral),
            Tok("0X8A", kHexLiteral),
            Tok("0xF", kHexLiteral),
            Tok("0XF", kHexLiteral),
            Err("0xG", LexerErrorType::UnexpectedSymbol),
            Err("0XZ", LexerErrorType::UnexpectedSymbol),
            Err("0x1z", LexerErrorType::UnexpectedSymbol),
            Tok("0xABCDEF0123456789", kHexLiteral),
            Tok("", kEOF),
        });
}

TEST(LexerTest, BinaryLiterals)
{
    CheckLexerOutput(
        std::source_location::current(),
        "0b0 0B0 0b10101 0b10102",
        {
            Tok("0b0", kBinaryLiteral),
            Tok("0B0", kBinaryLiteral),
            Tok("0b10101", kBinaryLiteral),
            Err("0b10102", LexerErrorType::UnexpectedSymbol),
            Tok("", kEOF),
        });
}

TEST(LexerTest, Comment)
{
    CheckLexerOutput(
        std::source_location::current(),
        R"(a b c // commented
               d e f)",
        {
            Tok("a", kIdentifier),
            Tok("b", kIdentifier),
            Tok("c", kIdentifier),
            Tok("// commented", TokenType::Comment),
            Tok("d", kIdentifier),
            Tok("e", kIdentifier),
            Tok("f", kIdentifier),
            Tok("", kEOF),
        });
}

TEST(LexerTest, BlockComment)
{
    CheckLexerOutput(
        std::source_location::current(),
        "abba b c /* d e f */ g h i /* j k l \n m n o */ p q r /* bla",
        {
            Tok("abba", kIdentifier),
            Tok("b", kIdentifier),
            Tok("c", kIdentifier),
            Tok("/* d e f */", TokenType::BlockComment),
            Tok("g", kIdentifier),
            Tok("h", kIdentifier),
            Tok("i", kIdentifier),
            Tok("/* j k l \n m n o */", TokenType::BlockComment),
            Tok("p", kIdentifier),
            Tok("q", kIdentifier),
            Tok("r", kIdentifier),
            Err("/* bla", LexerErrorType::UnterminatedBlockComment),
            Tok("", kEOF),
        });
}

TEST(LexerTest, Identifier)
{
    CheckLexerOutput(
        std::source_location::current(),
        "a ab ab_ _ _a _ab",
        {
            Tok("a", kIdentifier),
            Tok("ab", kIdentifier),
            Tok("ab_", kIdentifier),
            Tok("_", kIdentifier),
            Tok("_a", kIdentifier),
            Tok("_ab", kIdentifier),
            Tok("", kEOF),
        });
}

TEST(LexerTest, Keywords)
{
    CheckLexerOutput(
        std::source_location::current(),
        "def extern",
        {
            Tok("def", TokenType::Def),
            Tok("extern", TokenType::Extern),
            Tok("", kEOF),
        });
}

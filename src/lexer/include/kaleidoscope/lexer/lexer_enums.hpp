#pragma once

#include <cstdint>

namespace kaleidoscope
{

enum class TokenType : uint8_t
{
    Def,
    Extern,
    Identifier,
    FloatLiteral,
    DecimalLiteral,
    HexadecimalLiteral,
    BinaryLiteral,
    OctalLiteral,
    EndOfFile
};

enum class LexerErrorType : uint8_t
{
    UnexpectedSymbol,
    LeadingZeroInDecimalLiteral,
    ZeroLengthExponentInScientificNotation,
    NeedAtLeastOneDigitAroundDotInFloatLiteral,
    MultipleDotsInFloatingPointLiteral,
};
}  // namespace kaleidoscope

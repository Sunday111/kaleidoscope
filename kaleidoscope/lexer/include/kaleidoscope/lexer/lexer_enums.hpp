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
    StringLiteral,
    Comment,
    BlockComment,
    EndOfFile
};

enum class LexerErrorType : uint8_t
{
    UnexpectedSymbol,
    LeadingZeroInDecimalLiteral,
    ZeroLengthExponentInScientificNotation,
    NeedAtLeastOneDigitAroundDotInFloatLiteral,
    MultipleDotsInFloatingPointLiteral,
    UnterminatedBlockComment,
    MissingTerminatingCharacterForStringLiteral,
};

}  // namespace kaleidoscope

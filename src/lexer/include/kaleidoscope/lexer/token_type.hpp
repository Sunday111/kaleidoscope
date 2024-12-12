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
    IntegralLiteral,
    HexadecimalLiteral,
    ScientificNotationLiteral,
    EndOfFile
};
}  // namespace kaleidoscope

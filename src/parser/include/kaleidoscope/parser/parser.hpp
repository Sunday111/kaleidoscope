#include <variant>

#include "kaleidoscope/lexer/lexer.hpp"

namespace kaleidoscope
{

class ExprAST
{
public:
    constexpr virtual ~ExprAST() = default;
};

class IntegralLiteralExprAST : public ExprAST
{
public:
    uint64_t value = 0;
    uint8_t bits_count = 32;
    bool is_signed = true;
};

class FloatingPointLiteralExprAST : public ExprAST
{
public:
    std::variant<float, double> value;
};

class Parser
{
public:
};
}  // namespace kaleidoscope

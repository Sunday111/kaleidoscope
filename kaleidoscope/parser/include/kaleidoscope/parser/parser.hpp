#include <variant>

#include "kaleidoscope/lexer/lexer.hpp"

namespace kaleidoscope
{

class ExprAST
{
public:
    constexpr virtual ~ExprAST() = default;
};

enum class ExprType : uint8_t
{
    IntegralLiteral,
    BinaryOperator,
};

enum class ParserErrorType : uint8_t
{
    UnexpectedToken
};

class ExprId
{
public:
    ExprType type;
    uint32_t index;
};

using ExprASTResult = std::expected<ExprId, ParserErrorType>;

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

enum class BinaryOperatorType : uint8_t
{
    Plus,
    Minus,
    Multiply,
    Divide
};

class BinaryOperatorExpression : public ExprAST
{
public:
    ExprId left;
    ExprId right;
    BinaryOperatorType type;
};

}  // namespace kaleidoscope

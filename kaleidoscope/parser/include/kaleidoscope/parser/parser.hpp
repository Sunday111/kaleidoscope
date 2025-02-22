#include <variant>

#include "kaleidoscope/lexer/lexer.hpp"

namespace kaleidoscope
{

class TypeInfo
{
public:
};

enum class BuiltinType : uint8_t
{
    SignedInteger,
    UnsignedInteger,
    FloatingPoint
};

class BuiltinTypeInfo : public TypeInfo
{
public:
    BuiltinType type = BuiltinType::SignedInteger;
    uint8_t bits = 32;
};

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
    BuiltinTypeInfo type{};
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

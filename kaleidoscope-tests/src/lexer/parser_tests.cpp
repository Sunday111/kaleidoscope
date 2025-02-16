#include <bit>

#include "kaleidoscope/parser/parser.hpp"
#include "util.hpp"

namespace kaleidoscope
{

class Parser
{
public:
    template <size_t horizon_size>
    [[nodiscard]] constexpr ExprASTResult ParseDecimalIntegralLiteral(LookaheadLexer<horizon_size>& l)
    {
        LexerResult ra = l.Take();
        assert(ra.has_value());

        const LexerToken& ta = *ra;
        assert(ta.type == TokenType::DecimalLiteral);

        const std::string_view text = l.GetTokenView(ta);

        auto char_to_digit = [](char c) -> uint8_t
        {
            return std::bit_cast<uint8_t>(static_cast<int8_t>(c - '0'));
        };

        uint64_t value = 0;

        if (text.size() != 0)
        {
            value += char_to_digit(text.front());
            for (size_t i = 1; i != text.size(); ++i)
            {
                value = value * 10 + char_to_digit(text[i]);
            }
        }

        uint32_t index = static_cast<uint32_t>(integral_literals_.size());
        IntegralLiteralExprAST& expr = integral_literals_.emplace_back();
        expr.value = value;
        expr.bits_count = 64;
        expr.is_signed = false;

        return ExprId{
            .type = ExprType::IntegralLiteral,
            .index = index,
        };
    }

    template <size_t horizon_size>
    [[nodiscard]] constexpr ExprASTResult ParseExpression(LookaheadLexer<horizon_size>& l)
    {
        assert(l.Peek().has_value());

        auto first = [&]() -> ExprASTResult
        {
            switch (l.Peek()->type)
            {
            case TokenType::DecimalLiteral:
                return ParseDecimalIntegralLiteral(l);

            default:
                return std::unexpected(ParserErrorType::UnexpectedToken);
            }
        }();

        if (!first.has_value())
        {
            return first;
        }

        if (!l.Peek().has_value() || l.Peek()->type == TokenType::EndOfFile)
        {
            return first;
        }

        switch (l.Take()->type)
        {
        case TokenType::Plus:
        {
            auto second = ParseExpression(l);
            if (!second.has_value()) return second;

            const auto index = static_cast<uint32_t>(binary_operator_expression_.size());
            auto& expr = binary_operator_expression_.emplace_back();
            expr.left = *first;
            expr.right = *second;
            expr.type = BinaryOperatorType::Plus;
            return ExprId{
                .type = ExprType::BinaryOperator,
                .index = index,
            };
        }
        case TokenType::Minus:
        {
            auto second = ParseExpression(l);
            if (!second.has_value()) return second;

            const auto index = static_cast<uint32_t>(binary_operator_expression_.size());
            auto& expr = binary_operator_expression_.emplace_back();
            expr.left = *first;
            expr.right = *second;
            expr.type = BinaryOperatorType::Minus;
            return ExprId{
                .type = ExprType::BinaryOperator,
                .index = index,
            };
        }
        break;
        default:
            return std::unexpected(ParserErrorType::UnexpectedToken);
        }
    }

    template <ExprType type>
    [[nodiscard]] constexpr const auto* GetExprAst(uint32_t index) const
    {
        auto get_from = [&](auto&& v)
        {
            return index < v.size() ? &v[index] : nullptr;
        };

        if constexpr (type == ExprType::IntegralLiteral)
        {
            return get_from(integral_literals_);
        }
        else if constexpr (type == ExprType::BinaryOperator)
        {
            return get_from(binary_operator_expression_);
        }
        else
        {
            assert(false);
            return nullptr;
        }
    }

    std::vector<IntegralLiteralExprAST> integral_literals_;
    std::vector<BinaryOperatorExpression> binary_operator_expression_;
};
}  // namespace kaleidoscope

TEST(ParserTests, TwoLeadingZeroes)
{
    Lexer l("1234");
    LookaheadLexer<5> lexer(l);

    Parser parser;
    auto r = parser.ParseExpression(lexer);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r->type, ExprType::IntegralLiteral);

    auto* ast = parser.GetExprAst<ExprType::IntegralLiteral>(r->index);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->bits_count, 64);
    ASSERT_EQ(ast->is_signed, false);
    ASSERT_EQ(ast->value, 1234);
}

TEST(ParserTests, Plus)
{
    Lexer l("1 + 2");
    LookaheadLexer<5> lexer(l);

    Parser parser;
    auto r = parser.ParseExpression(lexer);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r->type, ExprType::BinaryOperator);

    auto* opt_ast = parser.GetExprAst<ExprType::BinaryOperator>(r->index);
    ASSERT_NE(opt_ast, nullptr);
    ASSERT_EQ(opt_ast->type, BinaryOperatorType::Plus);

    ASSERT_EQ(opt_ast->left.type, ExprType::IntegralLiteral);
    auto* left_ast = parser.GetExprAst<ExprType::IntegralLiteral>(opt_ast->left.index);
    ASSERT_NE(left_ast, nullptr);
    ASSERT_EQ(left_ast->bits_count, 64);
    ASSERT_EQ(left_ast->is_signed, false);
    ASSERT_EQ(left_ast->value, 1);

    ASSERT_EQ(opt_ast->right.type, ExprType::IntegralLiteral);
    auto* right_ast = parser.GetExprAst<ExprType::IntegralLiteral>(opt_ast->right.index);
    ASSERT_NE(right_ast, nullptr);
    ASSERT_EQ(right_ast->bits_count, 64);
    ASSERT_EQ(right_ast->is_signed, false);
    ASSERT_EQ(right_ast->value, 2);
}

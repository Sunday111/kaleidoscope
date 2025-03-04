#include <bit>
#include <cstdio>
#include <format>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#include "fmt/compile.h"
#pragma clang diagnostic pop
#include "ir_expression_executor.hpp"
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
        expr.type.bits = 32;
        expr.type.type = BuiltinType::SignedInteger;

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
    ASSERT_EQ(ast->type.bits, 32);
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
    ASSERT_EQ(left_ast->type.bits, 32);
    ASSERT_EQ(left_ast->value, 1);

    ASSERT_EQ(opt_ast->right.type, ExprType::IntegralLiteral);
    auto* right_ast = parser.GetExprAst<ExprType::IntegralLiteral>(opt_ast->right.index);
    ASSERT_NE(right_ast, nullptr);
    ASSERT_EQ(right_ast->type.bits, 32);
    ASSERT_EQ(right_ast->value, 2);
}

struct FixedBufferOutIt
{
    explicit constexpr FixedBufferOutIt(std::span<char> buffer)
        : size{buffer.size()},
          out{buffer.data()},
          end{out + buffer.size()}  // NOLINT
    {
    }

    constexpr FixedBufferOutIt& operator=(char c)
    {
        if (out != end)
        {
            *out = c;
            std::advance(out, 1);
        }

        size += 1;

        return *this;
    }

    constexpr FixedBufferOutIt& operator*() { return *this; }
    constexpr FixedBufferOutIt& operator++() { return *this; }
    constexpr FixedBufferOutIt operator++(int) { return *this; }

    size_t size = 0;
    char *out, *end{};
};

class CodeGen_LLVM_IR
{
public:
    constexpr explicit CodeGen_LLVM_IR(const Parser& parser, std::span<char> out, size_t first_variable_index = 0)
        : out_{out.data()},
          out_size_{std::ssize(out)},
          next_var_{first_variable_index},
          parser_{parser}
    {
    }

    template <typename... FormatArgs>
    constexpr void Write(fmt::format_string<FormatArgs...> format_string, FormatArgs&&... args)
    {
        const auto format_result =
            fmt::format_to_n(out_, static_cast<size_t>(out_size_), format_string, std::forward<FormatArgs>(args)...);

        out_size_ -= std::distance(out_, format_result.out);
        out_ = format_result.out;
        required_space_ += static_cast<size_t>(format_result.size);
    }

    // Returns variable index where result of expression will be stored
    [[nodiscard]] constexpr size_t Gen(const IntegralLiteralExprAST& literal)
    {
        assert(literal.type.bits == 32 || literal.type.bits == 64);

        const size_t var_ptr_id = next_var_++;
        const size_t var_id = next_var_++;
        const size_t align = literal.type.bits == 32 ? 4 : 8;

        Write("%{} = alloca i{}, align {}\n", var_ptr_id, literal.type.bits, align);
        Write("store i{} {}, ptr %{}, align {}\n", literal.type.bits, literal.value, var_ptr_id, align);
        Write("%{} = load i{}, ptr %{}, align {}\n", var_id, literal.type.bits, var_ptr_id, align);

        return var_id;
    }

    // Returns variable index where result of expression will be stored
    [[nodiscard]] constexpr size_t Gen(const BinaryOperatorExpression& binary_operator)
    {
        assert(binary_operator.left.type == ExprType::IntegralLiteral);
        assert(binary_operator.right.type == ExprType::IntegralLiteral);
        const size_t left = Gen(*parser_.GetExprAst<ExprType::IntegralLiteral>(binary_operator.left.index));
        const size_t right = Gen(*parser_.GetExprAst<ExprType::IntegralLiteral>(binary_operator.right.index));
        const size_t var_id = next_var_++;

        auto op = [&]() -> std::string_view
        {
            switch (binary_operator.type)
            {
            case BinaryOperatorType::Plus:
                return "add";
            case BinaryOperatorType::Minus:
                return "sub";
            default:
                assert(false);
                return "";
            }
        }();

        Write("%{} = {} i{} %{}, %{}\n", var_id, op, 32, left, right);

        return var_id;
    }

    char* out_;
    ssize_t out_size_;
    size_t next_var_ = 0;
    size_t required_space_ = 0;
    const Parser& parser_;  // NOLINT
};

[[nodiscard]] inline constexpr std::tuple<size_t, size_t> ExpressionToIR(
    std::string_view expression,
    std::span<char> ir)
{
    Lexer l(expression);
    LookaheadLexer<5> lexer(l);

    Parser parser;
    auto parse_result = parser.ParseExpression(lexer);
    auto* opt_ast = parser.GetExprAst<ExprType::BinaryOperator>(parse_result->index);
    CodeGen_LLVM_IR g{parser, ir, 1};
    size_t variable_index = g.Gen(*opt_ast);

    return {g.required_space_, variable_index};
}

static_assert(
    []
    {
        std::array<char, 2048> buffer{};
        constexpr auto f = FMT_COMPILE("{}");
        fmt::format_to(FixedBufferOutIt{std::span{buffer}}, f, 123);
        return true;
    }());

// static_assert(
//     []
//     {
//         std::array<char, 2048> buffer{};

//         return ExpressionToIR("42 - 21", buffer) == std::tuple<size_t, size_t>{192, 5};
//     }());

TEST(ParserTests, Gen)
{
    Lexer l("42 - 21");
    LookaheadLexer<5> lexer(l);

    Parser parser;
    auto r = parser.ParseExpression(lexer);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r->type, ExprType::BinaryOperator);

    auto* opt_ast = parser.GetExprAst<ExprType::BinaryOperator>(r->index);
    ASSERT_NE(opt_ast, nullptr);
    ASSERT_EQ(opt_ast->type, BinaryOperatorType::Minus);

    std::vector<char> data;
    data.resize(2048, 0);
    CodeGen_LLVM_IR g{parser, data, 1};
    size_t variable_index = g.Gen(*opt_ast);
    ASSERT_LE(g.required_space_, data.size());

    std::string var_name = std::format("%{}", variable_index);
    auto eval_result = IRExpressionExecutor::ExecI32(SpanAsStringView(std::span{data}), var_name);

    ASSERT_TRUE(eval_result.has_value());
    ASSERT_EQ(eval_result.value(), 21);
}

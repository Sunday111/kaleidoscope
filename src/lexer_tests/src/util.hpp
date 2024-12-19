#pragma once

// IWYU pragma: begin_exports
#include <ranges>
#include <source_location>

#include "gtest/gtest.h"
#include "kaleidoscope/lexer/lexer.hpp"
#include "kaleidoscope/lexer/lookahead_lexer.hpp"
#include "magic_enum/magic_enum.hpp"
using namespace kaleidoscope;  // NOLINT
// IWYU pragma: end_exports

inline constexpr auto kDecimalLiteral = TokenType::DecimalLiteral;
inline constexpr auto kHexLiteral = TokenType::HexadecimalLiteral;
inline constexpr auto kOctalLiteral = TokenType::OctalLiteral;
inline constexpr auto kFloatLiteral = TokenType::FloatLiteral;
inline constexpr auto kEOF = TokenType::EndOfFile;

[[nodiscard]] inline constexpr kaleidoscope::LexerToken EofToken(size_t text_len)
{
    return kaleidoscope::LexerToken{
        .type = kaleidoscope::TokenType::EndOfFile,
        .begin = text_len,
        .end = text_len,
    };
}

struct ExpectedToken
{
    constexpr bool operator==(const ExpectedToken&) const = default;

    std::string_view token;
    TokenType type;
};

struct ExpectedError
{
    constexpr bool operator==(const ExpectedError&) const = default;

    std::string_view token;
    LexerErrorType type;
};

using ExpectedResult = std::expected<ExpectedToken, ExpectedError>;

[[nodiscard]] inline constexpr ExpectedToken Tok(std::string_view text, TokenType type)
{
    return {
        text,
        type,
    };
}

[[nodiscard]] inline constexpr ExpectedResult Err(std::string_view text, LexerErrorType type)
{
    return std::unexpected(
        ExpectedError{
            .token = text,
            .type = type,
        });
}

inline constexpr void PrintExpectedResult(std::string_view prefix, const ExpectedResult& er, std::string_view suffix)
{
    std::print("{}", prefix);
    if (er.has_value())
    {
        std::print("{} {}", magic_enum::enum_name(er->type), er->token);
    }
    else
    {
        const auto& err = er.error();
        std::print("{}: {}", magic_enum::enum_name(err.type), err.token);
    }
    std::print("{}", suffix);
}

[[nodiscard]] constexpr ExpectedResult ToExpectedResult(std::string_view src, const LexerResult& r)
{
    if (r)
    {
        return ExpectedToken{
            .token = src.substr(r->begin, r->end - r->begin),
            .type = r->type,
        };
    }

    const auto& err = r.error();
    return std::unexpected(
        ExpectedError{
            .token = src.substr(err.begin, err.end - err.begin),
            .type = err.type,
        });
}

inline constexpr void CheckLexerOutput(
    std::source_location source_location,
    std::string_view src,
    std::initializer_list<ExpectedResult> expected_results)
{
    Lexer lexer(src);

    for (const size_t idx : std::views::iota(0uz, expected_results.size()))
    {
        const ExpectedResult& expected = expected_results.begin()[idx];  // NOLINT
        auto actual = ToExpectedResult(src, lexer.GetToken());
        if (expected != actual)
        {
            std::println("{}:{}", source_location.file_name(), source_location.line());
            std::println("At index {}:", idx);
            PrintExpectedResult("    Expected: ", expected, "\n");
            PrintExpectedResult("    Actual: ", actual, "\n");
            throw std::logic_error("Expectation mismatch");
        }
    }
}

template <size_t horizon_size>
inline constexpr void CheckLookaheadOutput(
    std::source_location source_location,
    std::string_view src,
    std::initializer_list<ExpectedResult> expected_results)
{
    Lexer lexer(src);
    LookaheadLexer<horizon_size> lookahead_lexer(lexer);

    for (const size_t base_idx : std::views::iota(size_t{0}, expected_results.size()))
    {
        for (const size_t offset :
             std::views::iota(size_t{0}, std::min(horizon_size, expected_results.size() - base_idx)))
        {
            const size_t peek_idx = base_idx + offset;
            const ExpectedResult& expected = expected_results.begin()[peek_idx];  // NOLINT
            auto actual = ToExpectedResult(src, lookahead_lexer.Peek(offset));
            if (expected != actual)
            {
                std::println("{}:{}", source_location.file_name(), source_location.line());
                std::println("At index {}:", peek_idx);
                PrintExpectedResult("    Expected: ", expected, "\n");
                PrintExpectedResult("    Actual: ", actual, "\n");
                throw std::logic_error("Expectation mismatch");
            }
        }

        [[maybe_unused]] auto token = lookahead_lexer.Take();
    }
}

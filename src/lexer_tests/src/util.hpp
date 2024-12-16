#pragma once

// IWYU pragma: begin_exports
#include <source_location>

#include "gtest/gtest.h"
#include "kaleidoscope/lexer/lexer.hpp"
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

inline constexpr void CheckLexerOutput(
    std::source_location source_location,
    std::string_view src,
    std::initializer_list<ExpectedResult> expected_results)
{
    Lexer lexer(src);
    size_t idx = 0;

    auto to_expected_result = [&](const LexerResult& r) -> ExpectedResult
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
    };

    for (const ExpectedResult& expected : expected_results)
    {
        auto actual = to_expected_result(lexer.GetToken());
        if (expected != actual)
        {
            std::println("{}:{}", source_location.file_name(), source_location.line());
            std::println("At index {}:", idx);
            PrintExpectedResult("    Expected: ", expected, "\n");
            PrintExpectedResult("    Actual: ", actual, "\n");
            throw std::logic_error("Expectation mismatch");
        }

        ++idx;
    }
}

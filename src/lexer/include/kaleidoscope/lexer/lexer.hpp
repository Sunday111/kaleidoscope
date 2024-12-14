#pragma once

#include <cassert>
#include <expected>
#include <print>
#include <string_view>

#include "ass/fixed_unordered_map.hpp"
#include "token_type.hpp"

namespace kaleidoscope
{

class LexerToken
{
public:
    [[nodiscard]] constexpr bool operator<=>(const LexerToken&) const noexcept = default;

    TokenType type = TokenType::EndOfFile;
    size_t begin = 0;
    size_t end = 0;
};

enum class LexerErrorType : uint8_t
{
    UnexpectedSymbol,
    LeadingZeroInIntegerLiteral,
    ZeroLengthSignificandInScientificNotation,
    ZeroLengthExponentInScientificNotation,
    NeedAtLeastOneDigitAroundDotInFloatLiteral,
};

class LexerError
{
public:
    [[nodiscard]] constexpr bool operator<=>(const LexerError&) const noexcept = default;

    LexerErrorType type;
    size_t pos;
};

using LexerResult = std::expected<LexerToken, LexerError>;

inline constexpr auto kKeywordLookup = []
{
    auto hasher = [](const std::string_view s) -> size_t
    {
        if (s.empty()) return 0;
        return std::bit_cast<uint8_t>(s.front());
    };

    ass::FixedUnorderedMap<10, std::string_view, TokenType, decltype(hasher)> m;

    auto add = [&m](std::string_view s, TokenType token_type)
    {
        assert(!m.Contains(s));
        m.Add(s, token_type);
    };

    add("def", TokenType::Def);
    add("extern", TokenType::Extern);

    return m;
}();

[[nodiscard]] constexpr auto BitsetFromChars(std::string_view s)
{
    ass::FixedBitset<256> b;

    for (char c : s)
    {
        const size_t idx = std::bit_cast<uint8_t>(c);
        assert(!b.Get(idx));
        b.Set(idx, true);
    }

    return b;
}

[[nodiscard]] constexpr auto BitsetFromCharRange(char first, char last)
{
    ass::FixedBitset<256> b;

    assert(first <= last);

    for (char c = first; c <= last; ++c)
    {
        b.Set(std::bit_cast<uint8_t>(c), true);
    }

    return b;
}

inline constexpr auto kLowerCaseLetters = BitsetFromCharRange('a', 'z');
inline constexpr auto kUpperCaseLetters = BitsetFromCharRange('A', 'Z');
inline constexpr auto kLetters = kLowerCaseLetters | kUpperCaseLetters;
inline constexpr auto kDigits = BitsetFromCharRange('0', '9');
inline constexpr auto kIdentifierHeadChars = kLetters | BitsetFromChars("_");
inline constexpr auto kIdentifierTailChars = kIdentifierHeadChars | kDigits;
inline constexpr auto kSpaceChars = BitsetFromChars(" \f\n\r\t\v");
inline constexpr auto kHexDigits = kDigits | BitsetFromCharRange('a', 'f') | BitsetFromCharRange('A', 'F');

class Lexer
{
public:
    [[nodiscard]] static constexpr bool OneOf(char c, const ass::FixedBitset<256>& b)
    {
        return b.Get(std::bit_cast<uint8_t>(c));
    }

    [[nodiscard]] static constexpr bool IsLetter(char c) { return OneOf(c, kLetters); }
    [[nodiscard]] static constexpr bool IsDigit(char c) { return OneOf(c, kDigits); }
    [[nodiscard]] static constexpr bool IsHexDigit(char c) { return OneOf(c, kHexDigits); }
    [[nodiscard]] static constexpr bool IsValidIdentifierHeadChar(char c) { return OneOf(c, kIdentifierHeadChars); }
    [[nodiscard]] static constexpr bool IsValidIdentifierTailChar(char c) { return OneOf(c, kIdentifierTailChars); }
    [[nodiscard]] static constexpr bool IsSpaceChar(char c) { return OneOf(c, kSpaceChars); }
    [[nodiscard]] static constexpr bool IsPossibleNumberCharacter(char c) { return OneOf(c, kSpaceChars); }

    constexpr explicit Lexer(std::string_view text) noexcept : text_(text) {}

    [[nodiscard]] constexpr LexerResult GetToken() noexcept
    {
        SkipSpaces();
        if (HasChars())
        {
            const char c = text_[pos_];
            if (IsLetter(c)) return ReadIdentifier();
            if (std::isdigit(c) || c == '.') return ReadNumber();

            return std::unexpected(
                LexerError{
                    .type = LexerErrorType::UnexpectedSymbol,
                    .pos = pos_,
                });
        }
        else
        {
            return LexerToken{
                .type = TokenType::EndOfFile,
                .begin = pos_,
                .end = pos_,
            };
        }
    }

private:
    [[nodiscard]] constexpr LexerResult ReadNumber() noexcept
    {
        // Assumes the current pos pointing at digit or at '.' symbol
        assert(HasChars());
        size_t begin = pos_;

        // Skip digits until meet something else
        auto skip_digits = [&]
        {
            while (HasChars() && IsDigit(text_[pos_])) ++pos_;
        };

        auto read_exponent = [&]() -> LexerResult
        {
            size_t e_pos = pos_++;

            // This case: .e3
            const size_t chars_before_e = e_pos - begin;
            if (chars_before_e == 1 && text_[e_pos - 1] == '.')
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::ZeroLengthSignificandInScientificNotation,
                        .pos = e_pos - 1,
                    });
            }

            skip_digits();

            // This case: 2e , 2.5e
            if (pos_ - e_pos == 1)
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::ZeroLengthExponentInScientificNotation,
                        .pos = e_pos,
                    });
            }

            return LexerToken{
                .type = TokenType::ScientificNotationLiteral,
                .begin = begin,
                .end = pos_,
            };
        };

        auto read_from_dot = [&] -> LexerResult
        {
            // it can be either regular float literal or scientific notation
            const size_t dot_pos = pos_++;

            skip_digits();

            if (HasChars() && (text_[pos_] == 'e' || text_[pos_] == 'E'))
            {
                return read_exponent();
            }

            if ((pos_ == dot_pos + 1) && begin == dot_pos)
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::NeedAtLeastOneDigitAroundDotInFloatLiteral,
                        .pos = dot_pos,
                    });
            }

            return LexerToken{
                .type = TokenType::FloatLiteral,
                .begin = begin,
                .end = pos_,
            };
        };

        if (text_[begin] == '.') return read_from_dot();

        assert(std::isdigit(text_[begin]));

        skip_digits();

        size_t int_length = pos_ - begin;
        size_t lead_zeroes = 0;
        for (size_t i = begin; i != pos_ && text_[i] == '0'; ++i)
        {
            ++lead_zeroes;
        }

        //  (octal numbers are not supported)
        if (lead_zeroes > 0 && int_length > 1)
        {
            return std::unexpected(
                LexerError{
                    .type = LexerErrorType::LeadingZeroInIntegerLiteral,
                    .pos = begin,
                });
        }

        if (HasChars() && text_[pos_] == '.')
        {
            return read_from_dot();
        }
        else if (HasChars() && (text_[pos_] == 'e' || text_[pos_] == 'E'))
        {
            return read_exponent();
        }
        else if (HasChars() && (text_[pos_] == 'X' || text_[pos_] == 'x'))
        {
            pos_++;

            while (HasChars() && IsHexDigit(text_[pos_])) ++pos_;

            return LexerToken{
                .type = TokenType::HexadecimalLiteral,
                .begin = begin,
                .end = pos_,
            };
        }
        else
        {
            // Met nothing but digits. Test the leading part does not start with zeroes
            return LexerToken{
                .type = TokenType::IntegralLiteral,
                .begin = begin,
                .end = pos_,
            };
        }
    }

    [[nodiscard]] constexpr LexerToken ReadIdentifier() noexcept
    {
        // Assumes the current pos not pointing at digit
        assert(HasChars() && IsValidIdentifierHeadChar(text_[pos_]));

        LexerToken token{
            .type = TokenType::Identifier,
            .begin = pos_,
        };
        ++pos_;
        while (HasChars() && IsValidIdentifierTailChar(text_[pos_]))
        {
            ++pos_;
        }

        token.end = pos_;

        const auto sub = text_.substr(token.begin, token.end - token.begin);
        if (const auto* pType = kKeywordLookup.Find(sub))
        {
            token.type = *pType;
        }

        return token;
    }

    [[nodiscard]] constexpr bool HasChars() const noexcept { return pos_ < text_.size(); }

    constexpr void SkipSpaces() noexcept
    {
        while (HasChars() && IsSpaceChar(text_[pos_]))
        {
            pos_++;
        }
    }

private:
    std::string_view text_;
    size_t pos_ = 0;
};

}  // namespace kaleidoscope

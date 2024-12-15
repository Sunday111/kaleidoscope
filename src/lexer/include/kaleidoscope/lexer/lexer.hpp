#pragma once

#include <cassert>
#include <expected>
#include <string_view>

#include "lexer_data.hpp"
#include "lexer_enums.hpp"

namespace kaleidoscope
{

class LexerToken
{
public:
    [[nodiscard]] constexpr bool operator==(const LexerToken&) const noexcept = default;

    TokenType type = TokenType::EndOfFile;
    size_t begin = 0;
    size_t end = 0;
};

class LexerError
{
public:
    [[nodiscard]] constexpr bool operator==(const LexerError&) const noexcept = default;

    LexerErrorType type;
    size_t pos;
};

using LexerResult = std::expected<LexerToken, LexerError>;

class Lexer
{
public:
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

    constexpr void SkipCurrent() noexcept
    {
        while (HasChars() && !IsSpaceChar(text_[pos_])) ++pos_;
    }

private:
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

    [[nodiscard]] constexpr LexerResult ReadFloatingPointNumber() noexcept
    {
        size_t begin = pos_;
        std::optional<size_t> dot;

        // Read just floating point number
        // Stop on exponent indicator, space or '_'.
        // Remember the position of the dot character.
        while (HasChars())
        {
            char c = text_[pos_];
            if (IsSpaceChar(c) || c == 'e' || c == 'E' || c == '_') break;

            if (c == '.')
            {
                if (dot)
                {
                    return std::unexpected(
                        LexerError{
                            .type = LexerErrorType::MultipleDotsInFloatingPointLiteral,
                            .pos = pos_,
                        });
                }

                dot = pos_;
            }

            ++pos_;
        }

        if (dot && begin + 1 == pos_)
        {
            return std::unexpected(
                LexerError{
                    .type = LexerErrorType::NeedAtLeastOneDigitAroundDotInFloatLiteral,
                    .pos = *dot,
                });
        }

        // Scientific notation
        if (HasChars() && (text_[pos_] == 'e' || text_[pos_] == 'E'))
        {
            size_t e = pos_++;
            while (HasChars() && IsDigit(text_[pos_])) ++pos_;

            if (pos_ - e == 1)
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::ZeroLengthExponentInScientificNotation,
                        .pos = e,
                    });
            }

            if (HasChars())
            {
                char c = text_[pos_];
                if (!IsSpaceChar(c) && c != '_')
                {
                    return std::unexpected(
                        LexerError{
                            .type = LexerErrorType::UnexpectedSymbol,
                            .pos = pos_,
                        });
                }
            }
        }

        return LexerToken{
            .type = TokenType::FloatLiteral,
            .begin = begin,
            .end = pos_,
        };
    }

    [[nodiscard]] constexpr LexerResult ReadDecimalNumber() noexcept
    {
        size_t begin = pos_;

        if (text_[pos_] == '0')
        {
            ++pos_;
            if (HasChars() && IsDigit(text_[pos_]))
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::LeadingZeroInDecimalLiteral,
                        .pos = pos_,
                    });
            }
        }

        while (HasChars() && IsDigit(text_[pos_])) ++pos_;

        if (HasChars())
        {
            char c = text_[pos_];
            if (!IsSpaceChar(c) && c != '_')
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::UnexpectedSymbol,
                        .pos = pos_,
                    });
            }
        }

        return LexerToken{
            .type = TokenType::DecimalLiteral,
            .begin = begin,
            .end = pos_,
        };
    }

    [[nodiscard]] constexpr LexerResult ReadHexNumber() noexcept
    {
        size_t begin = pos_;

        pos_ += 2;

        while (HasChars() && IsHexDigit(text_[pos_])) ++pos_;

        if (HasChars())
        {
            char c = text_[pos_];
            if (!IsSpaceChar(c) && c != '_')
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::UnexpectedSymbol,
                        .pos = pos_,
                    });
            }
        }

        return LexerToken{
            .type = TokenType::HexadecimalLiteral,
            .begin = begin,
            .end = pos_,
        };
    }

    [[nodiscard]] constexpr LexerResult ReadBinaryNumber() noexcept
    {
        size_t begin = pos_;
        pos_ += 2;

        static constexpr auto chars = BitsetFromChars("01");
        while (HasChars() && OneOf(text_[pos_], chars)) ++pos_;

        if (HasChars())
        {
            char c = text_[pos_];
            if (!IsSpaceChar(c) && c != '_')
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::UnexpectedSymbol,
                        .pos = pos_,
                    });
            }
        }

        return LexerToken{
            .type = TokenType::BinaryLiteral,
            .begin = begin,
            .end = pos_,
        };
    }

    [[nodiscard]] constexpr LexerResult ReadOctalNumber() noexcept
    {
        size_t begin = pos_;

        while (HasChars() && OneOf(text_[pos_], kOctalDigits)) ++pos_;

        if (HasChars())
        {
            char c = text_[pos_];
            if (!IsSpaceChar(c) && c != '_')
            {
                return std::unexpected(
                    LexerError{
                        .type = LexerErrorType::UnexpectedSymbol,
                        .pos = pos_,
                    });
            }
        }

        return LexerToken{
            .type = TokenType::OctalLiteral,
            .begin = begin,
            .end = pos_,
        };
    }

    [[nodiscard]] constexpr LexerResult ReadNumber() noexcept
    {
        assert(HasChars());

        size_t begin = pos_;

        if (IsDigit(text_[pos_]))
        {
            // Simple decimal numbers are the most common case
            auto decimal_result = ReadDecimalNumber();
            if (decimal_result.has_value())
            {
                return decimal_result;
            }
            else
            {
                pos_ = begin;
            }
        }

        if (text_[pos_] == '0')
        {
            char char_after_zero = text_[pos_ + 1];
            switch (char_after_zero)
            {
            case 'x':
            case 'X':
                return ReadHexNumber();
                break;
            case 'b':
            case 'B':
                return ReadBinaryNumber();
                break;
            }

            if (IsDigit(char_after_zero)) return ReadOctalNumber();
        }

        return ReadFloatingPointNumber();
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

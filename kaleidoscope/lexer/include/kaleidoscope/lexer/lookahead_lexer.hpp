#pragma once

#include <array>

#include "lexer.hpp"

namespace kaleidoscope
{

template <size_t horizon_size>
    requires(horizon_size >= 2)
class LookaheadLexer
{
public:
    constexpr explicit LookaheadLexer(Lexer& lexer) : lexer_(&lexer)
    {
        for (auto& slot : tokens_)
        {
            slot = lexer_->GetToken();
        }
    }

    [[nodiscard]] constexpr const LexerResult& Peek(size_t index = 0) noexcept
    {
        return tokens_[(start_index_ + index) % tokens_.size()];
    }

    [[nodiscard]] constexpr LexerResult Take() noexcept
    {
        LexerResult result = std::exchange(tokens_[start_index_], lexer_->GetToken());
        start_index_ = (start_index_ + 1) % tokens_.size();
        return result;
    }

    [[nodiscard]] constexpr std::string_view GetTokenView(const LexerToken& lexer_token) const
    {
        return lexer_->GetTokenView(lexer_token);
    }

private:
    Lexer* lexer_ = nullptr;
    std::array<LexerResult, horizon_size> tokens_;
    size_t start_index_ = 0;
};
}  // namespace kaleidoscope

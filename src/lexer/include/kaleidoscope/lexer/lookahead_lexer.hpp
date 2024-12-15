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
            if (!slot.has_value())
            {
                lexer_->SkipCurrent();
            }
        }
    }

    [[nodiscard]] constexpr const LexerResult& Peek(size_t index = 0) noexcept
    {
        return tokens_[(start_index_ + index) % tokens_.size()];
    }

    [[nodiscard]] constexpr LexerResult Take() noexcept
    {
        LexerResult token = std::move(tokens_[start_index_]);
        tokens_[start_index_] = lexer_->GetToken();
        start_index_ = (start_index_ + 1) % tokens_.size();
        return token;
    }

private:
    Lexer* lexer_ = nullptr;
    std::array<LexerResult, horizon_size> tokens_;
    size_t start_index_ = 0;
};
}  // namespace kaleidoscope

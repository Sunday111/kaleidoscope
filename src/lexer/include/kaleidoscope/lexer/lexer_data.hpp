#include <string_view>

#include "ass/fixed_unordered_map.hpp"
#include "lexer_enums.hpp"

namespace kaleidoscope
{

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
inline constexpr auto kOctalDigits = BitsetFromCharRange('0', '7');
}  // namespace kaleidoscope

#include <iostream>
#include <string>

#include "kaleidoscope/lexer/lexer.hpp"
#include "magic_enum/magic_enum.hpp"

int main()
{
    std::string s;

    std::getline(std::cin, s);

    kaleidoscope::Lexer lexer(s);

    auto t = lexer.GetToken();

    auto handle_error = [&]()
    {
        if (t.has_value()) return false;

        std::println("Error: {}", magic_enum::enum_name(t.error().type));
        return true;
    };

    while (!handle_error())
    {
        if (t->type == kaleidoscope::TokenType::EndOfFile)
        {
            std::println("EOF");
            break;
        }

        std::println(
            "{}, [{}, {}) = {}",
            magic_enum::enum_name(t->type),
            t->begin,
            t->end,
            std::string_view{s}.substr(t->begin, t->end - t->begin));

        t = lexer.GetToken();
    }
}

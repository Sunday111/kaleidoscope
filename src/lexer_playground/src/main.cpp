#include <iostream>
#include <string>

#include "kaleidoscope/lexer/lexer.hpp"
#include "magic_enum/magic_enum.hpp"

int main()
{
    std::string s;

    std::getline(std::cin, s);

    kaleidoscope::Lexer lexer(s);

    while (true)
    {
        auto t = lexer.GetToken();

        if (t.has_value())
        {
            std::println(
                "{}, [{}, {}) = {}",
                magic_enum::enum_name(t->type),
                t->begin,
                t->end,
                std::string_view{s}.substr(t->begin, t->end - t->begin));

            if (t->type == kaleidoscope::TokenType::EndOfFile)
            {
                std::println("EOF");
                break;
            }
        }
        else
        {
            auto& err = t.error();
            std::println(
                "{}, [{}, {}) = {}",
                magic_enum::enum_name(err.type),
                err.begin,
                err.end,
                std::string_view{s}.substr(err.begin, err.end - err.begin));
        }
    }
}

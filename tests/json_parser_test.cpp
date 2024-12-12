#include "json_parser.hpp"
#include <cassert>
#include <iostream>

using namespace hayk10002::json_parser::lexer;

int main() {

    // test Cursor
    {
        std::string text = 
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\n"
        "Aenean commodo .\n"
        "Aenean massa.\n"
        "Cum sociis .\n"
        "Donec ";

        Cursor c(text);
        assert(c.move(6) == "Lorem ");
        assert(c.move(-7) == "Lorem ");
        assert(c.move(66) == "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\nAenean c");
        assert(c.get_pos() == Position(66, 1, 8));
        assert(c.move(16) == "ommodo .\nAenean ");
        assert(c.next() == 'm');
        assert(c.get_pos() == Position(83, 2, 8));
        assert(c.move(-30) == "lit.\nAenean commodo .\nAenean m");
        assert(c.get_pos() == Position(53, 0, 53));
        assert(c.move(100000) == "lit.\nAenean commodo .\nAenean massa.\nCum sociis .\nDonec ");
        assert(c.get_pos() == Position(108, 4, 6));
        assert(c.next() == std::nullopt);
        c.set_pos(66);
        assert(c.get_pos() == Position(66, 1, 8));
    }

    // test Char, Digit, HexDigit parsers
    {
        int index = 0;
        std::function f = [&index](char x)
        { 
            return index < 4 && x == "true"[index++]; 
        };
        Cursor input{"true0fAlse"};
        CharParser chp(f);
        for (int i = 0; i < 4; i++) 
        {
            assert(chp.parse(input).value() == "true"[i]);
        }

        DigitParser dp;
        assert(dp.parse(input).value() == 0);
        assert(dp.parse(input).has_error());
        assert(input.next() == 'f');
        input.move(-1);

        HexDigitParser hdp;
        assert(hdp.parse(input).value() == 15);
        assert(hdp.parse(input).value() == 10);
        assert(hdp.parse(input).has_error());
        assert(input.next() == 'l');
        input.move(2);
    }

    // test TokenLiteralLexer
    {
        Cursor input{"true false Falseval null Null"};
        TokenLiteralLexer ll;
        assert(std::get<bool>(ll.parse(input).value().value) == true);
        input.next();
        assert(std::get<bool>(ll.parse(input).value().value) == false);
        input.next();
        auto res = ll.parse(input);
        assert(res.has_error()); std::cout << res.error().what() << std::endl;
        input.move(9);
        assert(ll.parse(input).value().value.index() == 0);
        input.next();
        res = ll.parse(input);
        assert(res.has_error()); std::cout << res.error().what() << std::endl;
        input.move(4);
    }

    // test TokenSyntaxLexer
    {
        Cursor input{"{[:,]}./"};
        TokenSyntaxLexer ll;
        assert(ll.parse(input).value().type == TokenSyntax::OBJECT_START);
        assert(ll.parse(input).value().type == TokenSyntax::ARRAY_START);
        assert(ll.parse(input).value().type == TokenSyntax::SEMICOLON);
        assert(ll.parse(input).value().type == TokenSyntax::COMMA);
        assert(ll.parse(input).value().type == TokenSyntax::ARRAY_END);
        assert(ll.parse(input).value().type == TokenSyntax::OBJECT_END);
        auto res = ll.parse(input);
        assert(res.has_error()); std::cout << res.error().what() << std::endl;
        input.next();
        res = ll.parse(input);
        assert(res.has_error()); std::cout << res.error().what() << std::endl;
        input.next();
        input.next();
        assert(input.next() == std::nullopt);
    }

    // test TokenNumberLexer
    {
        Cursor good_input{
                "1\n"
                "2\n"
                "10\n"
                "-10\n"
                "-0\n"
                "0\n"
                "1.0\n"
                "2.00\n"
                "-4.00\n"
                "9223372036854775807\n"
                "-9223372036854775808\n"
                "9223372036854775808\n"
                "-9223372036854775809\n"
                "24e10\n"
                "24.3550E-4\n"
                "0.123e+234\n"
                "1e1000\n"
                "1e-1000"};

        TokenNumberLexer num_l{};
        CharParser new_line_p{'\n'};
        auto tokens = hayk10002::parser_types::Cycle{num_l, new_line_p}.parse(good_input).value();
        for (const auto& token: tokens)
        {
            std::cout << (token.value.index() ? "double" : "int64") << ' ';
            std::visit([](const auto& arg){ std::cout << arg; }, token.value);
            std::cout << std::endl;
        }
        auto bad_inputs = { "1ea", "", "a", "-4. ", "0.3e- ", "234.e3" }; // "02" will not be an error, since a 0 will be parsed successfully
        for (auto bad_input: bad_inputs)
        {
            Cursor input{bad_input};
            auto err = num_l.parse(input);
            assert(err.has_error());
            std::cout << err.error().what() << std::endl;
        }
    }

    //test TokenStringLexer
    {
        Cursor good_input{
            R"("Hello, world!")"
            R"("JSON allows UTF-8 😊")"
            R"("This is a \"quoted\" string.")"
            R"("Path to file: C:\\Users\\Example")"
            R"("Line one\nLine two")"
            R"("Emoji: 😊")"
            R"("Chinese: 中文")"
            R"("Arabic: العربية")"
            R"("Devanagari: हिन्दी")"
            R"("Mathematical symbols: ∑ ∆ ∞")"
            R"("\u4F60\u597D\uD83D\uDE00\uD834\uDD1E\u26A1")"
            R"("Miscellaneous: \u263A\u2665\u26A1")"
            R"("Musical note: \uD834\uDD1E")"
        };

        TokenStringLexer str_l;
        auto tokens = hayk10002::parser_types::Cycle{str_l}.parse(good_input).value();
        for (const auto& token: tokens) 
            std::cout << token.value << std::endl;

        auto bad_inputs = 
        {
            "\"Invalid (raw surrogate pair): \xED\xA0\xBD\xED\xB8\x80\"",
            "\"Invalid (unescaped control char): hello\u0009world\"",
            "\"Invalid (unescaped backslash): C:\\path\\to\\file\"",
            "\"Invalid (lone high surrogate): \\uD83D\"",
            "\"Invalid (lone low surrogate): \\uDFFF\"",
            "\"Invalid (invalid escape): \\x41\"",
            "\"Unexpected end of input"
        };

        for (auto bad_input: bad_inputs)
        {
            Cursor input{bad_input};
            auto err = str_l.parse(input);
            assert(err.has_error());
            std::cout << err.error().what() << std::endl;
        }
    }

    return 0;
}

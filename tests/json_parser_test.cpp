#include "json_parser.hpp"
#include "cassert"

using hayk10002::json_parser::lexer::Cursor;
using hayk10002::json_parser::lexer::Position;
using hayk10002::json_parser::lexer::CharParser;
using hayk10002::json_parser::lexer::DigitParser;
using hayk10002::json_parser::lexer::HexDigitParser;

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
    }

    return 0;
}

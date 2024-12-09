#include "json_parser.hpp"
#include "cassert"

using hayk10002::json_parser::lexer::Cursor;

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

    return 0;
}

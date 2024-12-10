#include "parser_error.hpp"
#include <cassert>
#include <string>

using hayk10002::json_parser::lexer::Position;
using hayk10002::json_parser::lexer::UnexpectedCharacter;
using hayk10002::json_parser::lexer::UnexpectedEndOfInput;
using hayk10002::json_parser::lexer::InvalidLiteral;

int main() {

    assert(UnexpectedCharacter(Position(15, 4, 3), 'a', "a digit").what() == std::string("Unexpected character ('a') at line: 4, col: 3  (pos: 15). Expected a digit."));
    assert(UnexpectedCharacter(Position(15, 4, 3), 'b').what() == std::string("Unexpected character ('b') at line: 4, col: 3  (pos: 15)."));
    assert(UnexpectedEndOfInput(Position(15, 4, 3)).what() == std::string("Unexpected end of input at line: 4, col: 3  (pos: 15)."));
    assert(InvalidLiteral(Position(15, 4, 3), "asdf").what() == std::string("Invalid literal (\"asdf\") at line: 4, col: 3  (pos: 15). Expected \"null\", \"true\" or \"false\"."));

    return 0;
}

#include "parser_error.hpp"
#include <cassert>
#include <string>

using hayk10002::json_parser::lexer::Position;
using hayk10002::json_parser::lexer::UnexpectedCharacter;
using hayk10002::json_parser::lexer::UnexpectedEndOfInput;

int main() {

    assert(UnexpectedCharacter(Position(15, 4, 3), "a digit").what() == std::string("Unexpected character at line: 4, col: 3  (pos: 15). Expected a digit."));
    assert(UnexpectedCharacter(Position(15, 4, 3)).what() == std::string("Unexpected character at line: 4, col: 3  (pos: 15)."));
    assert(UnexpectedEndOfInput(Position(15, 4, 3)).what() == std::string("Unexpected end of input at line: 4, col: 3  (pos: 15)."));

    return 0;
}

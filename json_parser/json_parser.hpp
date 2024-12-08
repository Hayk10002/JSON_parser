#pragma once

#include <string>
#include <vector>
#include <format>
#include <optional>
#include <functional>

#include "position.hpp"
#include "parser_error.hpp"
#include "parser.hpp"

namespace hayk10002::json_parser::lexer
{
    class Cursor
    {

    private:
        std::string_view m_view{};
        /// stores already encountered line starts, except first one, for every line start the previous character is '\n'
        /// first value always 0
        std::vector<size_t> m_line_starts{0};
        Position m_pos{};

    public:
        Cursor(std::string_view view): m_view(view) {}

        /// @brief moves the cursor forward or backward, and returns selected part
        ///
        /// "hel_lo world" -> move(3) -> "hello _world" and returns "lo "
        ///
        /// "hello wo_rld" -> move(-5) -> "hel_lo world" and returns "lo wo"
        ///
        /// if moving will place cursor out of bounds, it is placed exactly at start or at end
        /// @param d how much to move the cursor
        /// @return selected part
        std::string_view move(int d)
        {
            // if moving d will go out of bounds, recalculate d
            if (int(m_pos) + d < 0) d = -m_pos;
            else if (m_pos + d >= m_view.size()) d = m_view.size() - m_pos;

            std::string_view res_span{};
            
            // if moving forward
            if (d >= 0) 
            {
                res_span = m_view.substr(m_pos, d);

                // while not on last line, and if moving will change the line
                while(m_pos.line < m_line_starts.size() - 1 && m_line_starts[m_pos.line + 1] <= m_pos + d) 
                {
                    // move to the start of next line
                    d -= m_line_starts[m_pos.line + 1] - m_pos;
                    m_pos.pos = m_line_starts[m_pos.line + 1];
                    m_pos.line++;
                    m_pos.col = 0;
                }
                // not on the last line, but will stay on this line 
                if (m_pos.line != m_line_starts.size() - 1)
                {
                    m_pos.pos += d;
                    m_pos.col = d;
                }
                // on last line
                else
                {
                    while(d)
                    {
                        // encounter new line and register it
                        if (m_view[m_pos] == '\n')
                        {
                            m_pos.line++;
                            m_pos.pos++;
                            m_pos.col = 0;
                            m_line_starts.push_back(m_pos);
                        }
                        else 
                        {
                            m_pos.pos++;
                            m_pos.col++;
                        }
                        d--;
                    }
                }
            }
            // if moving backward
            if (d < 0) 
            {
                res_span = m_view.substr(m_pos + d, -d);
                m_pos.pos += d;
                // find the line that the cursor will be after moving
                while(m_line_starts[m_pos.line] > m_pos) m_pos.line--;
                m_pos.col = m_pos - m_line_starts[m_pos.line];
            }
            return res_span;
        }


        Position get_pos() { return m_pos; }
        void set_pos(size_t pos) { move(pos - m_pos); }
        void set_pos(Position pos) { set_pos(pos.pos); }

        /// @brief get next character
        /// @return return std::nullopt if at the end, or the next character
        std::optional<char> next()
        {
            if (auto v = move(1); !v.empty()) return v[0];
            else return std::nullopt;
        }

    };

    /// @brief parses a character if provided function returns true for that character
    class CharParser
    {
    public:
        using InputType = Cursor;
        using ReturnType = char;
        using ErrorType = ParserError<UnexpectedCharacter, UnexpectedEndOfInput>;

    private:
        std::function<bool(char)> m_accept;
        std::string m_expected_text{};

    public:
        /// @param accept_char function that takes char, returns bool, indicates what characters are accepted, and what are not
        /// @param expected_text optional text to print if the character is not accepte by the function (error message will say  "Unexpected character at {some position}. Expected {expected_text}.")
        /// @note if expected_text is empty, the "Expected {expected_text}" part will not appear in the error message at all
        CharParser(std::function<bool(char)> accept_char, std::string_view expected_text = ""): m_accept(accept_char), m_expected_text(expected_text) {}

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            // get next character from input
            auto ch = input.next();

            // if successfull
            if (ch)
            {
                // check if the character is accepted by the provided function
                if (m_accept(*ch)) return std::move(*ch);
                else 
                {
                    // error unexpected character, move cursor one place back, because in case of error parsers are expected to not change the input
                    input.move(-1);
                    return itlib::unexpected(UnexpectedCharacter(input.get_pos(), m_expected_text));
                }
            } 
            // no next character (end of input)
            else 
            {
                // error unexpected end of input, move cursor one place back, because in case of error parsers are expected to not change the input
                input.move(-1);
                return itlib::unexpected(UnexpectedEndOfInput(input.get_pos()));
            }
        }
    };

    /// @brief parses a digit (characters from 0 to 9), returns number values (for '0' returns 0, not '0')
    class DigitParser
    {
    public:
        using InputType = Cursor;
        using ReturnType = int;
        using ErrorType = ParserError<UnexpectedCharacter, UnexpectedEndOfInput>;

    private:
        CharParser m_chp;

    public:
        DigitParser(): m_chp([](char ch) { return ch >= '0' && ch <= '9'; }, "a digit (from 0 to 9)"){}
        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            auto res = m_chp.parse(input);
            if (res.has_error()) return itlib::unexpected(res.error());
            return res.value() - '0';
        }
    };

    /// @brief parses a hes digit (characters from 0 to 9, a to f, A to F), returns number values (for '0' returns 0, not '0')
    class HexDigitParser
    {
    public:
        using InputType = Cursor;
        using ReturnType = int;
        using ErrorType = ParserError<UnexpectedCharacter, UnexpectedEndOfInput>;

    private:
        CharParser m_chp;

    public:
        HexDigitParser(): m_chp([](char ch) { return ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f'; }, "a hex digit (from 0 to 9, a to f or A to F)"){}
        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            auto res = m_chp.parse(input);
            if (res.has_error()) return itlib::unexpected(res.error());
            char ch = res.value();
            if (ch >= '0' && ch <= '9') return ch - '0';
            if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
            if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
            // unreachable
            return -1;
        }
    };
}
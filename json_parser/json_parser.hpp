#pragma once

#include <string>
#include <vector>
#include <format>
#include <optional>
#include <functional>
#include <limits>
#include <cmath>
#include <iostream>

#include "position.hpp"
#include "parser_error.hpp"
#include "parser.hpp"
#include "json.hpp"

namespace hayk10002::json_parser
{
    using namespace parser_types;
namespace lexer 
{
    /// @brief token for literal values
    struct TokenLiteral { std::variant<Json::NullType, Json::BoolType> value; };

    /// @brief token for number values
    struct TokenNumber { std::variant<Json::IntType, Json::FloatType> value; };

    /// @brief token for string values
    struct TokenString { Json::StringType value; };

    /// @brief token for syntax related characters
    struct TokenSyntax
    { 
        enum Type: char
        {
            COMMA = ',',
            COLON = ':',
            ARRAY_START = '[',
            ARRAY_END = ']',
            OBJECT_START = '{',
            OBJECT_END = '}',
        } type;
    };

    struct Token
    {
        std::variant<TokenLiteral, TokenNumber, TokenString, TokenSyntax> inner;
        Position pos;
        bool is_literal() const { return std::holds_alternative<TokenLiteral>(inner); }
        bool is_number()  const { return std::holds_alternative<TokenNumber>(inner); }
        bool is_string()  const { return std::holds_alternative<TokenString>(inner); }
        bool is_syntax(TokenSyntax::Type type = TokenSyntax::Type{0}) const 
        { 
            return std::holds_alternative<TokenSyntax>(inner) && ((type != 0) == (std::get<TokenSyntax>(inner).type == type)); 
        }

        TokenLiteral&       get_literal()       { return std::get<TokenLiteral>(inner); }
        TokenNumber&        get_number()        { return std::get<TokenNumber>(inner); }
        TokenString&        get_string()        { return std::get<TokenString>(inner); }
        TokenSyntax&        get_syntax()        { return std::get<TokenSyntax>(inner); }
        
        const TokenLiteral& get_literal() const { return std::get<TokenLiteral>(inner); }
        const TokenNumber&  get_number()  const { return std::get<TokenNumber>(inner); }
        const TokenString&  get_string()  const { return std::get<TokenString>(inner); }
        const TokenSyntax&  get_syntax()  const { return std::get<TokenSyntax>(inner); }

        friend std::ostream& operator<<(std::ostream& out, const Token& token)
        {
            if (token.is_literal())
            {
                auto t = token.get_literal().value;
                if (t.index() == 0) out << "null";
                else out << (std::get<1>(t) ? "true" : "false");
            }
            else if (token.is_number())
            {
                auto t = token.get_number().value;
                std::visit([&out](const auto& arg){ out << arg; }, t);
            }
            else if (token.is_string())
                out << '"' << token.get_string().value << '"';
            else
                out << '\'' << char(token.get_syntax().type) << '\'';

            return out;
        }
    };

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

        /// @brief returns the part that would be selected if the cursor moved by d
        ///
        /// "hel_lo world" -> peek(3) -> returns "lo "
        ///
        /// "hello wo_rld" -> peek(-5) -> returns "lo wo"
        ///
        /// if moving would place cursor out of bounds, it returns up to start or end
        /// @param d how much to peek and in what direction
        std::string_view peek(int d) const
        {
            // if moving d will go out of bounds, recalculate d
            if (int(m_pos) + d < 0) d = -m_pos;
            else if (m_pos + d >= m_view.size()) d = m_view.size() - m_pos;

            return (d >= 0) ? m_view.substr(m_pos, d) : m_view.substr(m_pos + d, -d);
        }


        Position get_pos() const { return m_pos; }
        void set_pos(size_t pos) { move(pos - m_pos); }
        void set_pos(Position pos) { set_pos(pos.pos); }

        /// @brief get next character
        /// @return return std::nullopt if at the end, or the next character
        /// @note this moves the cursor
        std::optional<char> next()
        {
            if (auto v = move(1); !v.empty()) return v[0];
            else return std::nullopt;
        }

        /// @brief get next character without moving the cursor
        /// @return return std::nullopt if at the end, or the next character
        /// @note this doesn't move the cursor
        std::optional<char> peek_next() const
        {
            if (auto v = peek(1); !v.empty()) return v[0];
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

        /// @param accepted_char character to accept
        /// @param expected_text optional text to print if the character is not accepte by the function (error message will say  "Unexpected character at {some position}. Expected {expected_text}.")
        /// @note if expected_text is empty, the "Expected {expected_text}" part will not appear in the error message at all
        CharParser(char accepted_char, std::string_view expected_text = ""): CharParser([accepted_char](char ch){ return ch == accepted_char; }, expected_text) {}

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
                    return itlib::unexpected(UnexpectedCharacter(input.get_pos(), *ch, m_expected_text));
                }
            } 
            // no next character (end of input)
            else return itlib::unexpected(UnexpectedEndOfInput(input.get_pos()));
        }
    };

    /// @brief parses a digit (characters from 0 to 9), returns number values (for '0' returns 0, not '0')
    class DigitParser
    {
    public:
        using InputType = Cursor;
        using ReturnType = int;
        using ErrorType = ParserError<ExpectedADigit, UnexpectedEndOfInput>;

    private:
        CharParser m_chp;

    public:
        DigitParser(): m_chp([](char ch) { return ch >= '0' && ch <= '9'; }){}
        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            auto res = m_chp.parse(input);
            if (res.has_error())
            { 
                auto err = res.error();
                if (std::holds_alternative<UnexpectedEndOfInput>(err.inner)) return itlib::unexpected(err);
                return itlib::unexpected(ExpectedADigit(std::get<UnexpectedCharacter>(err.inner).pos, std::get<UnexpectedCharacter>(err.inner).found));
            }
            return res.value() - '0';
        }
    };

    /// @brief parses a hes digit (characters from 0 to 9, a to f, A to F), returns number values (for '0' returns 0, not '0')
    class HexDigitParser
    {
    public:
        using InputType = Cursor;
        using ReturnType = int;
        using ErrorType = ParserError<ExpectedAHexDigit, UnexpectedEndOfInput>;

    private:
        CharParser m_chp;

    public:
        HexDigitParser(): m_chp([](char ch) { return ch >= '0' && ch <= '9' || ch >= 'A' && ch <= 'F' || ch >= 'a' && ch <= 'f'; }){}
        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            auto res = m_chp.parse(input);
            if (res.has_error())
            { 
                auto err = res.error();
                if (std::holds_alternative<UnexpectedEndOfInput>(err.inner)) return itlib::unexpected(err);
                return itlib::unexpected(ExpectedAHexDigit(std::get<UnexpectedCharacter>(err.inner).pos, std::get<UnexpectedCharacter>(err.inner).found));
            }
            char ch = res.value();
            if (ch >= '0' && ch <= '9') return ch - '0';
            if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
            if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
            // unreachable
            return -1;
        }
    };

    /// @brief parses a utf-8 codepoint from input, returns the string and the codepoint
    class UTF8CodePointParser
    {
    public:
        using InputType = Cursor;
        using ReturnType = std::tuple<UTF8string, uint32_t>;
        using ErrorType = ParserError<InvalidEncoding, UnexpectedEndOfInput>;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            char ch;
            // if input is empty
            if (auto maybe_ch = input.peek_next(); !maybe_ch) 
               return itlib::unexpected(UnexpectedEndOfInput{input.get_pos()});
            else ch = *maybe_ch;

            uint32_t codepoint = 0;
            size_t numBytes = 0;

            // Determine the number of bytes in the UTF-8 sequence
            if ((ch & 0b10000000) == 0) {
                // 1-byte sequence: 0xxxxxxx
                codepoint = ch;
                numBytes = 1;
            } else if ((ch & 0b11100000) == 0b11000000) {
                // 2-byte sequence: 110xxxxx 10xxxxxx
                codepoint = ch & 0b00011111;
                numBytes = 2;
            } else if ((ch & 0b11110000) == 0b11100000) {
                // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
                codepoint = ch & 0b00001111;
                numBytes = 3;
            } else if ((ch & 0b11111000) == 0b11110000) {
                // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                codepoint = ch & 0b00000111;
                numBytes = 4;
            } else {
                return itlib::unexpected(InvalidEncoding{input.get_pos(), "Invalid UTF-8 start byte"});
            }

            // Verify the continuation bytes
            auto bytes = input.peek(numBytes);
            if (bytes.size() < numBytes) {
                return itlib::unexpected(InvalidEncoding{input.get_pos(), "Input string is too short for a valid UTF-8 codepoint"});
            }

            for (size_t i = 1; i < numBytes; ++i) {
                if ((bytes[i] & 0b11000000) != 0b10000000) {
                    return itlib::unexpected(InvalidEncoding{input.get_pos(), "Invalid UTF-8 continuation byte"});
                }
                codepoint = (codepoint << 6) | (bytes[i] & 0b00111111);
            }

            // Detect overlong encoding
            if ((numBytes == 2 && codepoint <= 0x7F) ||
                (numBytes == 3 && codepoint <= 0x7FF) ||
                (numBytes == 4 && codepoint <= 0xFFFF)) {
                return itlib::unexpected(InvalidEncoding{input.get_pos(), "Overlong UTF-8 encoding"});
            }

            // Validate codepoint range (optional, depending on your requirements)
            if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                return itlib::unexpected(InvalidEncoding{input.get_pos(), "Invalid UTF-8 codepoint"});
            }

            input.move(numBytes);
            return std::tuple{UTF8string(std::string(bytes)), codepoint};
        }

    };

    /// @brief parses a literal ('null', 'true' or 'false')
    class TokenLiteralLexer
    {
    public:
        using InputType = Cursor;
        using ReturnType = TokenLiteral;
        using ErrorType = ParserError<ExpectedALiteral, InvalidLiteral, UnexpectedEndOfInput>;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            Position start_pos = input.get_pos();

            // read letters, and store them in val
            std::string val = "";

            // stores true if reached end of input
            bool end_of_input = true;

            // read a character while can
            while (auto ch = input.next())
            {
                // if a letter, add to val
                if (std::isalpha(*ch)) val += *ch;

                // else set end_of_input false, and stop reading
                else { end_of_input = false; break; }
            }

            // if reached end of input but read nothing
            if (end_of_input && val == "") return itlib::unexpected(UnexpectedEndOfInput(start_pos));

            // if not reached end of input, then one character was read after the literal's end, so unread it
            if (!end_of_input) input.move(-1);

            if (val == "") return itlib::unexpected(ExpectedALiteral(start_pos));

            // check for accepted literals
            if (val == "true") return TokenLiteral{true};
            if (val == "false") return TokenLiteral{false};
            if (val == "null") return TokenLiteral{};

            // in case of failure set input to starting pos
            input.set_pos(start_pos);

            return itlib::unexpected(InvalidLiteral(start_pos, val));
        }
    };

    class TokenSyntaxLexer
    {
    public:
        using InputType = Cursor;
        using ReturnType = TokenSyntax;
        using ErrorType = ParserError<ExpectedASyntax, UnexpectedEndOfInput>;

    private:
        CharParser m_chp;

    public:
        TokenSyntaxLexer(): m_chp([](char ch) 
            { 
                return 
                    ch == TokenSyntax::COMMA ||
                    ch == TokenSyntax::COLON ||
                    ch == TokenSyntax::ARRAY_START ||
                    ch == TokenSyntax::ARRAY_END ||
                    ch == TokenSyntax::OBJECT_START ||
                    ch == TokenSyntax::OBJECT_END; 
            }, "a syntax character (',', ':', '[', ']', '{' or '}')"){}
        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            auto res = m_chp.parse(input);
            if (res.has_error())
            {
                auto err = res.error();
                if (std::holds_alternative<UnexpectedEndOfInput>(err.inner)) return itlib::unexpected(err);
                return itlib::unexpected(ExpectedASyntax(input.get_pos()));
            }
            return TokenSyntax{TokenSyntax::Type{res.value()}};
        }
    };

    class TokenNumberLexer
    {
    public:
        using InputType = Cursor;
        using ReturnType = TokenNumber;
        using ErrorType = ParserError<ExpectedANumber, ExpectedADigit, ExpectedADigitOrASign, UnexpectedEndOfInput>;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            Position start_pos = input.get_pos();
            CharParser e_parser([](char ch) { return ch == 'e' || ch == 'E'; });
            DigitParser digit_parser;
            Cycle digits_parser{digit_parser};

            // If next character does not exist, or it is not a digit or a minus sign, return an error
            if (auto ch = input.peek_next(); !ch || (*ch != '-' && !std::isdigit(*ch))) 
            {
                if (!ch) return itlib::unexpected(UnexpectedEndOfInput{start_pos});
                return itlib::unexpected(ExpectedANumber{start_pos});
            };

            // Try parsing a minus sign, if successfull, then is_negative is true
            bool is_negative = Or{CharParser('-'), Nothing<InputType>{}}.parse(input).value().index() == 0;
            int sign = (!is_negative - is_negative); // 0 -> (1 - 0) = 1, 1 -> (0 - 1) = -1

            // Parse the first digit, in case of failure, return an error
            auto res = digit_parser.parse(input);
            if (res.has_error()) 
            {
                input.set_pos(start_pos);
                return itlib::unexpected(res.error());
            }

            int first_digit = res.value();

            // Hold two values, in case if the input can be represented by an integer, and in case it needs to be represented by floating point value
            Json::FloatType float_value = first_digit; float_value *= sign;
            Json::IntType int_value = first_digit * sign;
            bool is_int = true;

            if (first_digit != 0) 
            {
                // If the first digit is not 0, parse other digits in the whole part of the number
                const Json::IntType max = std::numeric_limits<Json::IntType>::max();
                const Json::IntType min = std::numeric_limits<Json::IntType>::min();
                auto digits = digits_parser.parse(input).value();
                for (int digit: digits)
                {
                    // for every digit, multiply by 10 and add the digit, for integer check for overflows
                    float_value *= Json::FloatType{10};
                    float_value += Json::FloatType(digit * sign);
                    if (is_int)
                    {
                        // if overflow after multiplying with 10, switch to float
                        if (int_value > max / 10 || int_value < min / 10) is_int = false;
                        else int_value *= 10;

                        // if overflow after adding next digit, switch to float
                        if (sign == 1 && int_value > max - digit || sign == -1 && int_value < min + digit) is_int = false;
                        else int_value += digit * sign;
                    }
                }
            }

            auto fraction_parser = Seq{CharParser('.'), digit_parser, digits_parser};
            auto sign_parser = Or{CharParser('-'), CharParser('+'), Nothing<InputType>{}};
            auto exponent_parser = Seq{e_parser, sign_parser, digit_parser, digits_parser};

            int final_exp = 0;

            auto res1 = fraction_parser.parse(input);

            // if parsing floating point part failed, and failed parsing the first digit after the point, return an error
            if (res1.has_error() && res1.error().index() == 1)
            {
                input.set_pos(start_pos);
                return itlib::unexpected(std::get<1>(res1.error()));
            }

            // if successfully parsed floating point part
            if (res1.has_value())
            {
                // definetely not integer
                is_int = false;
                auto [_dot, first_digit, digits] = res1.value();
                digits.insert(digits.begin(), first_digit);

                // for every digit multiply by 10 and add the digit, but decrease the exponent
                for (int digit: digits)
                {
                    final_exp--;
                    float_value *= Json::FloatType{10};
                    float_value += Json::FloatType(sign * digit);
                }
            }

            int exp = 0;
            bool exp_out_of_limits = false;

            auto res2 = exponent_parser.parse(input);

            // if failed parsing the exponent part by failing parsing the first digit after the character e or E, return an error
            if (res2.has_error() && res2.error().index() == 2)
            {
                auto err = std::get<2>(res2.error());
                input.set_pos(start_pos);
                if (std::get<1>(exponent_parser.get_info())->index() == 2 && std::holds_alternative<ExpectedADigit>(err.inner)) 
                    return itlib::unexpected(ExpectedADigitOrASign(std::get<ExpectedADigit>(err.inner).pos, std::get<ExpectedADigit>(err.inner).found));
                return itlib::unexpected(std::get<2>(res2.error()));
            }

            // if successfully parsed the exponent part
            if (res2.has_value())
            {
                is_int = false;
                auto [_dot, exp_sign_var, first_digit, digits] = res2.value();
                int exp_sign = exp_sign_var.index() == 0; // true for negative
                exp_sign = !exp_sign - exp_sign; // -1 or 1
                digits.insert(digits.begin(), first_digit);

                // for every digit multiply the exponent with 10 and add the digit 
                for (int digit: digits)
                {
                    exp *= 10;
                    exp += exp_sign * digit;

                    // check if exponent is too big or too small for the type, and make the number infinity or 0 respectively
                    if (exp > std::numeric_limits<Json::FloatType>::max_exponent10 * 2)
                    {
                        float_value = std::numeric_limits<Json::FloatType>::infinity() * sign;
                        exp_out_of_limits = true;
                        break;
                    }
                    else if (exp < std::numeric_limits<Json::FloatType>::min_exponent10 * 2)
                    {
                        float_value = 0 * sign;
                        exp_out_of_limits = true;
                        break;
                    }
                }

                final_exp += exp;                
            }

            if (!exp_out_of_limits) 
            {
                // apply the exponent
                while(final_exp > 0)
                {
                    float_value *= Json::FloatType{10};
                    final_exp--;
                }
                while(final_exp < 0)
                {
                    float_value /= Json::FloatType{10};
                    final_exp++;
                }
            }

            // return integer or float
            if (is_int) return TokenNumber{Json::IntType{int_value}};
            return TokenNumber{Json::FloatType{float_value}};
        }
    };

    class TokenStringLexer
    {
    public:
        using InputType = Cursor;
        using ReturnType = TokenString;
        using ErrorType = ParserError<ExpectedAString, InvalidEncoding, UnexpectedControlCharacter, InvalidEscape, UnexpectedEndOfInput>;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            // if next character doesn't exist, return error
            if (auto ch = input.peek_next(); !ch) 
                return itlib::unexpected(UnexpectedEndOfInput{input.get_pos()});

            // if next character isn't a double quote, retrun error
            else if (*ch != '\"')
                return itlib::unexpected(ExpectedAString{input.get_pos()});

            Position start_pos = input.get_pos();
            
            // skip the starting double quote
            input.next();

            std::string result = "";
            UTF8CodePointParser utf8_p{};
            CharParser escape_char_p([](char ch)
                {
                    return
                        ch == '\\' ||
                        ch == '\"' ||
                        ch == '/' ||
                        ch == 'b' ||
                        ch == 'f' ||
                        ch == 'n' ||
                        ch == 'r' ||
                        ch == 't' ||
                        ch == 'u';
                });

            while(true)
            {
                Position curr_pos = input.get_pos();

                // parse a utf-8 codepoint
                auto res = utf8_p.parse(input);

                // if failed, return the error
                if (res.has_error())
                {
                    input.set_pos(start_pos);
                    return itlib::unexpected(res.error());
                }
                auto [utf8_str, codepoint] = res.value();

                // if encountered a double quote, end the cycle
                if (codepoint == '\"') break;

                // if encountered a control character, return error
                if (codepoint < 32)
                {
                    input.set_pos(start_pos);
                    return itlib::unexpected(UnexpectedControlCharacter{curr_pos, char(codepoint)});
                }

                // if anything other then backslash for escape, add it to the result and continue
                if (codepoint != '\\')
                {
                    result += utf8_str.utf8_sstring();
                    continue;
                }

                // if got a backslash, means we need to parse an escape sequence
                // parse the first character after the backslash
                auto esc_res = escape_char_p.parse(input);

                // if not a valid escape, return an error
                if (esc_res.has_error())
                { 
                    input.set_pos(start_pos);
                    auto err = esc_res.error();
                    if (std::holds_alternative<UnexpectedEndOfInput>(err.inner)) return itlib::unexpected(err);
                    return itlib::unexpected(InvalidEscape(curr_pos, std::string{'\\', std::get<UnexpectedCharacter>(err.inner).found}));
                }

                char esc_ch = esc_res.value();
                
                // check the escape character
                switch (esc_ch)
                {
                case '\\':
                    result += '\\';
                    break;
                case '\"':
                    result += '\"';
                    break;
                case '/':
                    result += '/';
                    break;
                case 'b':
                    result += '\b';
                    break;
                case 'f':
                    result += '\f';
                    break;
                case 'n':
                    result += '\n';
                    break;
                case 'r':
                    result += '\r';
                    break;
                case 't':
                    result += '\t';
                    break;
                case 'u':
                    // if the character is 'u', need to parse 4 hex digits
                    HexDigitParser hp{};
                    auto res = Seq{hp, hp, hp, hp}.parse(input);

                    // if failed parsing, return error
                    if (res.has_error())
                    {
                        input.move(-2);
                        std::string_view esc_str = input.peek(6);
                        input.set_pos(start_pos);
                        return itlib::unexpected(InvalidEscape(curr_pos, esc_str));
                    }

                    auto [a, b, c, d] = res.value();

                    // calculate the value, treat the value as utf16 code unit
                    uint16_t val1 = (a << 12) + (b << 8) + (c << 4) + d;

                    auto is_low_surrogate  = [](uint16_t val) { return (val & 0b1111110000000000) == 0b1101110000000000; };
                    auto is_high_surrogate = [](uint16_t val) { return (val & 0b1111110000000000) == 0b1101100000000000; };

                    auto utf16_to_utf8 = [](uint16_t val1, uint16_t val2)
                    {
                        uint32_t codepoint;
                        if (val2 == 0) {
                            // Single UTF-16 unit
                            codepoint = val1;
                        } else {
                            // Surrogate pair: high surrogate in `val1`, low surrogate in `val2`
                            codepoint = 0x10000 + (((val1 & 0x03FF) << 10) | (val2 & 0x03FF));
                        }

                        std::string utf8;
                        if (codepoint <= 0x7F) {
                            // 1-byte UTF-8
                            utf8.push_back(static_cast<char>(codepoint));
                        } else if (codepoint <= 0x7FF) {
                            // 2-byte UTF-8
                            utf8.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
                            utf8.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        } else if (codepoint <= 0xFFFF) {
                            // 3-byte UTF-8
                            utf8.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
                            utf8.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                            utf8.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        } else if (codepoint <= 0x10FFFF) {
                            // 4-byte UTF-8
                            utf8.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
                            utf8.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
                            utf8.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
                            utf8.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
                        }
                        return utf8;
                    };

                    // if it is not a low or high surrogate, than it is a valid utf16 codepoint, convert to utf-8 and add to the result
                    if (!is_low_surrogate(val1) && !is_high_surrogate(val1))
                    {
                        result += utf16_to_utf8(val1, 0);
                        break;
                    }

                    // if it is a low surrogate (low surrogates are expected after a high surrogate, that is not present), return error
                    if (is_low_surrogate(val1))
                    {
                        input.set_pos(start_pos);
                        return itlib::unexpected(InvalidEncoding(curr_pos, "Low surrogate not after a high surrogate", "utf-16"));
                    }

                    // if it is a high surrogate, check the next two characters, because there needs to be a low surrogate after it
                    // if not "\u", return error
                    if (input.peek(2) != "\\u")
                    {
                        input.set_pos(start_pos);
                        return itlib::unexpected(InvalidEncoding(curr_pos, "High surrogate not before a low surrogate", "utf-16"));
                    }

                    // skip the "\u"
                    input.move(2);

                    // parse 4 hex characters
                    res = Seq{hp, hp, hp, hp}.parse(input);

                    // if failed return error
                    if (res.has_error())
                    {
                        input.move(-2);
                        curr_pos = input.get_pos();
                        std::string_view esc_str = input.peek(6);
                        input.set_pos(start_pos);
                        return itlib::unexpected(InvalidEscape(curr_pos, esc_str));
                    }

                    auto [e, f, g, h] = res.value();

                    // calculate the value
                    uint16_t val2 = (e << 12) + (f << 8) + (g << 4) + h;

                    // if not a low surrogate return error
                    if (!is_low_surrogate(val2))
                    {
                        input.set_pos(start_pos);
                        return itlib::unexpected(InvalidEncoding(curr_pos, "High surrogate not before a low surrogate", "utf-16"));
                    }

                    // convert the surrogate pair to utf-8 and add to result
                    result += utf16_to_utf8(val1, val2);

                    // break for switch case
                    break;
                }
            }

            return TokenString{Json::StringType{result}};
        }

    };

    /// @brief A simple "parser" to get the current input position
    struct PositionGetter
    {
        using InputType = Cursor;
        using ReturnType = Position;
        using ErrorType = NoError;
        itlib::expected<ReturnType, ErrorType> parse(const InputType& input) { return input.get_pos(); }
    };

    class JsonLexer
    {
        bool m_to_the_end_of_input;
    public:
        using InputType = Cursor;
        using ReturnType = std::vector<Token>;
        using ErrorType = ParserError<UnexpectedCharacter, InvalidLiteral, ExpectedADigit, ExpectedADigitOrASign, InvalidEncoding, UnexpectedControlCharacter, InvalidEscape, UnexpectedEndOfInput>;
        
        /// @param to_the_end_of_input is it required to parse the input entirely, or can stop if encountering unexpected token
        JsonLexer(bool to_the_end_of_input = true): m_to_the_end_of_input(to_the_end_of_input){}    
        
        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            Or token_l{TokenLiteralLexer{}, TokenNumberLexer{}, TokenStringLexer{}, TokenSyntaxLexer{}};
            Seq pos_and_token{PositionGetter{}, token_l};
            Cycle whitespace_p{CharParser{[](char ch) { return ch == '\t' || ch == '\n' || ch == '\r' || ch == ' '; }}};

            Position start_pos = input.get_pos();

            // skip whitespace at the start of input
            whitespace_p.parse(input);

            // parse tokens as much as possible
            Cycle tokens_l{pos_and_token, whitespace_p};
            auto result = tokens_l.parse(input).value();

            // skip whitespace at the end of input
            whitespace_p.parse(input);

            // store resulted tokens
            std::vector<Token> tokens(result.size());
            for (size_t i = 0; i < tokens.size(); i++) 
            {
                auto [pos, token] = std::move(result[i]);
                tokens[i] = Token{std::move(token), pos};
            }

            // if there is no need to parse the input entirely, return the result
            if (!m_to_the_end_of_input) return tokens;

            // get errors, because of which the parsing stopped
            auto [lit_err, num_err, str_err, syn_err] = std::get<1>(std::get<0>(tokens_l.get_info()));

            // if the error is not an UnexpectedEndOfInput
            if (!std::holds_alternative<UnexpectedEndOfInput>(lit_err.inner) ||
                !std::holds_alternative<UnexpectedEndOfInput>(num_err.inner) ||
                !std::holds_alternative<UnexpectedEndOfInput>(str_err.inner) ||
                !std::holds_alternative<UnexpectedEndOfInput>(syn_err.inner))
            {
                // if the error is not a ExpectedALiteral, ExpectedANumber, etc., because if an error is like that, it means the token parser wasn't able to start parsing the new token
                // so if token was partially parsed, return the error gotten from the respective token parser

                if (lit_err.inner.index() != 0)
                {
                    input.set_pos(start_pos);
                    return itlib::unexpected(lit_err);
                }

                if (num_err.inner.index() != 0)
                {
                    input.set_pos(start_pos);
                    return itlib::unexpected(num_err);
                }

                if (str_err.inner.index() != 0)
                {
                    input.set_pos(start_pos);
                    return itlib::unexpected(str_err);
                }

                if (syn_err.inner.index() != 0)
                {
                    input.set_pos(start_pos);
                    return itlib::unexpected(syn_err);
                }
            }

            // if input is not at the end, return error
            if (input.peek_next()) 
            {
                Position curr_pos = input.get_pos();
                char found = *input.next();
                input.set_pos(start_pos);
                return itlib::unexpected(UnexpectedCharacter(curr_pos, found, "a literal, a number, a string, or a syntax character"));
            }

            return tokens;
        }
    };
}

    /// @brief Cursor for a span of const T
    /// @note requires for type T to have a public member named pos with type Position
    template<class T>
        requires requires(T t) { requires std::same_as<std::decay_t<decltype(t.pos)>, Position>; }
    class SpanCursor
    {
        std::span<const T> span;
        size_t pos;
        Position end_pos;
        
    public:
        size_t get_pos() const { return pos; }
        void set_pos(size_t pos) { this->pos = pos; }

        /// @param sp the span to place the cursor on
        /// @param end_pos store the text position of the end of the span
        SpanCursor(std::span<T> sp, Position end_pos): span{sp}, pos{}, end_pos{end_pos} {}

        /// get next element of span, returns nullptr if at the end of the span, moves the cursor
        const T* next()       { if (pos < span.size()) return &span[pos++]; else return nullptr; }

        // see next element of span, returns nullptr if at the end of the span, does not move the cursor
        const T* peek() const { if (pos < span.size()) return &span[pos  ]; else return nullptr; }

        // get the text position of the cursor, if at the end of span, returns the provided end_pos
        const Position& get_text_curr_pos() const { return peek() ? peek()->pos : get_text_end_pos(); }

        // get the text position at the end of the span
        const Position& get_text_end_pos()  const { return end_pos; }

        operator std::span<const T>() { return span; }
    };

    using namespace json_parser::lexer;

    /// @brief Parse a json object from a span of tokens
    class JsonParserFromTokens
    {
    public:
        using InputType = SpanCursor<Token>;
        using ReturnType = Json;
        using ErrorType = ParserError<ExpectedAValue, ExpectedAStringOrObjectEnd, ExpectedColon, ExpectedCommaOrObjectEnd, ExpectedAString, ExpectedAValueOrArrayEnd, ExpectedCommaOrArrayEnd>;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input);
    };

    /// @brief Parse a literal from a span of tokens
    class LiteralParser
    {
    public:
        using InputType = SpanCursor<Token>;
        using ReturnType = Json;
        using ErrorType = ExpectedALiteral;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            Position curr_pos = input.get_text_curr_pos();
            if (!input.peek() || !input.peek()->is_literal()) return itlib::unexpected(ExpectedALiteral{curr_pos});
            Token token = *input.next();
            Json res;
            if (std::holds_alternative<Json::NullType>(token.get_literal().value)) return Json::null();
            return Json::boolean(std::get<Json::BoolType>(std::move(token.get_literal().value)));
        }
    };

    /// @brief Parse a number from a span of tokens
    class NumberParser
    {
    public:
        using InputType = SpanCursor<Token>;
        using ReturnType = Json;
        using ErrorType = ExpectedANumber;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            Position curr_pos = input.get_text_curr_pos();
            if (!input.peek() || !input.peek()->is_number()) return itlib::unexpected(ExpectedANumber{curr_pos});
            Token token = *input.next();
            Json res;
            if (std::holds_alternative<Json::IntType>(token.get_number().value)) return Json::number_int(std::get<Json::IntType>(std::move(token.get_number().value)));
            return Json::number_float(std::get<Json::FloatType>(std::move(token.get_number().value)));
        }
    };

    /// @brief Parse a string from a span of tokens
    class StringParser
    {
    public:
        using InputType = SpanCursor<Token>;
        using ReturnType = Json;
        using ErrorType = ExpectedAString;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            Position curr_pos = input.get_text_curr_pos();
            if (!input.peek() || !input.peek()->is_string()) return itlib::unexpected(ExpectedAString{curr_pos});
            return Json::string(input.next()->get_string().value);
        }
    };

    /// @brief Parse a syntax character of an exact type from a span of tokens
    class SyntaxParser
    {
    public:
        using InputType = SpanCursor<Token>;
        using ReturnType = TokenSyntax::Type;
        using ErrorType = ExpectedASyntax;

    private:
        TokenSyntax::Type m_syn_type;

    public:
        SyntaxParser(TokenSyntax::Type syn_type) : m_syn_type{syn_type} {}

        itlib::expected<ReturnType, ErrorType> parse(InputType& input)
        {
            Position curr_pos = input.get_text_curr_pos();
            if (!input.peek() || !input.peek()->is_syntax() || input.peek()->get_syntax().type != m_syn_type) return itlib::unexpected(ExpectedASyntax{curr_pos});
            return TokenSyntax::Type{input.next()->get_syntax().type};
        }
    };

    /// @brief Parse a json array from a span of tokens
    class ArrayParser
    {
    public:
        using InputType = SpanCursor<Token>;
        using ReturnType = Json;
        using ErrorType = ParserError<ExpectedArrayStart, ExpectedAStringOrObjectEnd, ExpectedColon, ExpectedCommaOrObjectEnd, ExpectedAString, ExpectedAValueOrArrayEnd, ExpectedCommaOrArrayEnd>;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input);
    };

    itlib::expected<ArrayParser::ReturnType, ArrayParser::ErrorType> ArrayParser::parse(ArrayParser::InputType& input)
    {
        JsonParserFromTokens val_p{};
        Cycle values_p{val_p, SyntaxParser{TokenSyntax::COMMA}};

        size_t start_pos = input.get_pos();

        // parse array opening bracket, values, and a closing bracket
        auto res = Seq{SyntaxParser{TokenSyntax::ARRAY_START}, values_p, SyntaxParser{TokenSyntax::ARRAY_END}}.parse(input);

        // if failed
        if (res.has_error())
        {
            input.set_pos(start_pos);
            auto err = res.error();

            // if failed parsing the opening bracket
            if (err.index() == 0) return itlib::unexpected(ExpectedArrayStart{std::get<0>(err).pos});

            // failed parsing the closing bracket, and before that failed parsing a comma
            if (err.index() == 2 && values_p.get_info().index() == 1) return itlib::unexpected(ExpectedCommaOrArrayEnd{std::get<2>(err).pos});

            // failed parsing the closing bracket, and before that failed parsing a value
            if (err.index() == 2 && values_p.get_info().index() == 0)
            {
                auto err = std::get<0>(values_p.get_info());

                // if wasn't being able to parse event the start of a json value
                if (err.inner.index() == 0) return itlib::unexpected(ExpectedAValueOrArrayEnd{std::get<0>(err.inner).pos});

                // if started parsing a value and failed not at start, return the error gotten from value parser
                return itlib::unexpected(err);
            } 
        }

        return Json::array(std::move(std::get<1>(res.value())));
    }

    /// @brief Parse a json object from a span of tokens
    class ObjectParser
    {
    public:
        using InputType = SpanCursor<Token>;
        using ReturnType = Json;
        using ErrorType = ParserError<ExpectedObjectStart, ExpectedAStringOrObjectEnd, ExpectedColon, ExpectedCommaOrObjectEnd, ExpectedAString, ExpectedAValueOrArrayEnd, ExpectedCommaOrArrayEnd, ExpectedAValue>;

        itlib::expected<ReturnType, ErrorType> parse(InputType& input);
    };

    itlib::expected<ObjectParser::ReturnType, ObjectParser::ErrorType> ObjectParser::parse(ObjectParser::InputType& input)
    {
        StringParser key_p{};
        JsonParserFromTokens val_p{};

        Seq key_value_p{key_p, SyntaxParser{TokenSyntax::COLON}, val_p};

        Cycle keys_values_p{key_value_p, SyntaxParser{TokenSyntax::COMMA}};

        size_t start_pos = input.get_pos();

        // parse object opening bracket, then key value pairs, then closing bracket
        auto res = Seq{SyntaxParser{TokenSyntax::OBJECT_START}, keys_values_p, SyntaxParser{TokenSyntax::OBJECT_END}}.parse(input);

        // if failed
        if (res.has_error())
        {
            input.set_pos(start_pos);
            auto err = res.error();

            // if failed parsing the opening bracket
            if (err.index() == 0) return itlib::unexpected(ExpectedObjectStart{std::get<0>(err).pos});

            // if failed parsing the closing bracket, and before that failed parsing a comma
            if (err.index() == 2 && keys_values_p.get_info().index() == 1) return itlib::unexpected(ExpectedCommaOrObjectEnd{std::get<2>(err).pos});

            // if failed parsing the closing bracket, and befroe that failed parsing a key value pair 
            if (err.index() == 2 && keys_values_p.get_info().index() == 0)
            {
                auto err = std::get<0>(keys_values_p.get_info());

                // if failed parsing the key (a string)
                if (err.index() == 0) return itlib::unexpected(ExpectedAStringOrObjectEnd{std::get<0>(err).pos});

                // if failed parsing the colon between the key and the value 
                if (err.index() == 1) return itlib::unexpected(ExpectedColon{std::get<1>(err).pos});

                // if failed parsing the value
                if (err.index() == 2) 
                {
                    auto error = std::get<2>(err);

                    // if wasn't able to parse even the start of a value
                    if (error.inner.index() == 0) return itlib::unexpected(ExpectedAValue{std::get<0>(error.inner).pos});

                    // if started parsing a value and failed not at start, return the error gotten from value parser
                    return itlib::unexpected(error);
                }
            } 
        }
        auto [_s, keys_and_values, _e] = std::move(res.value());

        // construct the json object from the data gotten from the parser
        Json::ObjectType result{};
        for (auto it = keys_and_values.begin(); it != keys_and_values.end(); it++)
        {
            result[std::get<0>(*it).get_string()] = std::get<2>(*it);
        }
        return Json::object(std::move(result));
    }

    /// @brief parse a json value from token span
    itlib::expected<JsonParserFromTokens::ReturnType, JsonParserFromTokens::ErrorType> JsonParserFromTokens::parse(InputType &input)
    {
        Or p{LiteralParser{}, NumberParser{}, StringParser{}, ArrayParser{}, ObjectParser{}};
        auto res = p.parse(input);

        // if success, return the value
        if (res.has_value()) return std::visit([](Json&& val){ return std::move(val); }, std::move(res.value()));

        // get errors
        auto [lit_err, num_err, str_err, arr_err, obj_err] = res.error();

        // if array or object failed parsing midway, return the returned error
        if (arr_err.inner.index() != 0) return itlib::unexpected(arr_err);
        if (obj_err.inner.index() != 0) return itlib::unexpected(obj_err);

        return itlib::unexpected(ExpectedAValue{input.get_text_curr_pos()});
    }
}

#pragma once

#include <concepts>
#include <exception>
#include <variant>
#include <string>
#include <format>

#include "position.hpp"
#include "utils.hpp"

namespace hayk10002
{
    template<std::derived_from<std::exception> ...ErrorTypes>
    struct ParserError : public std::exception
    {
        std::variant<ErrorTypes...> inner;
        
        virtual const char* what() const noexcept override
        {
            return std::visit([this](const auto& arg){ return arg.what(); }, inner);
        }

        template<std::derived_from<std::exception> ...ETs>
        ParserError(const ParserError<ETs...>& other): inner(cast_variant<ErrorTypes...>(other.inner)) {}

        template<std::derived_from<std::exception> ...ETs>
        ParserError(ParserError<ETs...>&& other): inner(cast_variant<ErrorTypes...>(std::move(other.inner))) {}

        template<std::derived_from<std::exception> ET>
        ParserError(ET&& other): inner(std::forward<ET>(other)) {}
    };

    namespace json_parser
    {
    namespace lexer
    {
        class UnexpectedCharacter : public std::exception
        {
        protected:
            std::string m_message;
        
        public:
            char found;
            Position pos;
            UnexpectedCharacter(const Position& pos, char found, std::string_view expected_text = ""):
                found(found),
                pos(pos),
                m_message(
                        expected_text == "" ? 
                        std::format("Unexpected character ('{}') at line: {}, col: {}  (pos: {}).", found, pos.line, pos.col, pos.pos) : 
                        std::format("Unexpected character ('{}') at line: {}, col: {}  (pos: {}). Expected {}.", found, pos.line, pos.col, pos.pos, expected_text)
                )
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class UnexpectedControlCharacter : public UnexpectedCharacter
        {
        public:
            inline static std::string_view control_character_names[32] = 
            {
                "NUL",	
                "SOH",	
                "STX",	
                "ETX",	
                "EOT",	
                "ENQ",	
                "ACK",	
                "BEL",	
                "BS",	
                "HT",	
                "LF",	
                "VT",	
                "FF",	
                "CR",	
                "SO",	
                "SI",	
                "DLE",
                "DC1",
                "DC2",
                "DC3",
                "DC4",
                "NAK",
                "SYN",
                "ETB",
                "CAN",
                "EM",
                "SUB",
                "ESC",
                "FS",
                "GS",
                "RS",
                "US"
            };

            UnexpectedControlCharacter(const Position& pos, char found): UnexpectedCharacter(pos, found) 
            {
                if (0 <= found && found < 32)
                {
                    std::string_view name = control_character_names[found];
                    const char escape[2] = {char('0' + (found > 15)), char((found % 16 > 9) ? ('a' + found % 16 - 9) : ('0' + found % 16))};

                    m_message = std::format("Unexpected control character ({}) at line: {}, col: {}, (pos: {}). It must be escaped with \"\\u00{}\".", name, pos.line, pos.col, pos.pos, escape);
                }
            }
        };

        class UnexpectedEndOfInput : public std::exception
        {
            std::string m_message;

        public:
            UnexpectedEndOfInput(const Position& pos):
                m_message(std::format("Unexpected end of input at line: {}, col: {}  (pos: {}).", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedADigit : public UnexpectedCharacter
        {
        public:
            ExpectedADigit(const Position& pos, char found): UnexpectedCharacter(pos, found, "a digit (from 0 to 9)") {}
        };

        class ExpectedAHexDigit : public UnexpectedCharacter
        {
        public:
            ExpectedAHexDigit(const Position& pos, char found): UnexpectedCharacter(pos, found, "a hex digit (from 0 to 9, a to f or A to F)") {}
        };

        class InvalidEncoding : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            InvalidEncoding(const Position& pos, std::string_view details = "", std::string_view encoding = "utf-8"):
                pos(pos),
                m_message(
                        details == "" ? 
                        std::format("Invalid {} at line: {}, col: {}  (pos: {}).", encoding, pos.line, pos.col, pos.pos) : 
                        std::format("Invalid {} at line: {}, col: {}  (pos: {}). {}.", encoding, pos.line, pos.col, pos.pos, details)
                )
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedADigitOrASign : public UnexpectedCharacter
        {
        public:
            ExpectedADigitOrASign(const Position& pos, char found): UnexpectedCharacter(pos, found, "a digit (from 0 to 9) or a sign (- or +)") {}
        };

        class InvalidLiteral : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            std::string found;
            InvalidLiteral(const Position& pos, std::string_view found):
                found(found),
                pos(pos),
                m_message(std::format("Invalid literal (\"{}\") at line: {}, col: {}  (pos: {}). Expected \"null\", \"true\" or \"false\".", found, pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class InvalidEscape : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            std::string found;
            InvalidEscape(const Position& pos, std::string_view found):
                found(found),
                pos(pos),
                m_message(std::format("Invalid escape (\"{}\") at line: {}, col: {}  (pos: {}). Allowed escapes are \"\\\"\", \"\\\\\", \"\\/\", \"\\b\", \"\\f\", \"\\n\", \"\\r\", \"\\t\", \"\\uhhhh\" where h is a hex digit (0 to 9, a to f or A to F).", found, pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedALiteral : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedALiteral(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a literal (\"null\", \"true\" or \"false\") at line: {}, col: {}  (pos: {}).", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedANumber : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedANumber(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a number at line: {}, col: {}  (pos: {}).", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedAString : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedAString(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a string at line: {}, col: {}  (pos: {}) (strings start and end with the \" character).", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedASyntax : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedASyntax(const Position& pos):
                pos(pos),
                m_message(std::format("Expecte a syntax character (',', ':', '[', ']', '{{' or '}}') at line: {}, col: {}  (pos: {}).", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };
    }

        using json_parser::lexer::Position;

        class ExpectedArrayStart : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedArrayStart(const Position& pos):
                pos(pos),
                m_message(std::format("Expected '[' at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedObjectStart : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedObjectStart(const Position& pos):
                pos(pos),
                m_message(std::format("Expected '{{' at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };
        class ExpectedAValue : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedAValue(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a value at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedAStringOrObjectEnd : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedAStringOrObjectEnd(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a string or '}}' at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedColon : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedColon(const Position& pos):
                pos(pos),
                m_message(std::format("Expected ':' at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedCommaOrObjectEnd : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedCommaOrObjectEnd(const Position& pos):
                pos(pos),
                m_message(std::format("Expected ',' or '}}' at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedAValueOrArrayEnd : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedAValueOrArrayEnd(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a value or ']' at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };
        
        class ExpectedCommaOrArrayEnd : public std::exception
        {
            std::string m_message;
        
        public:
            Position pos;
            ExpectedCommaOrArrayEnd(const Position& pos):
                pos(pos),
                m_message(std::format("Expected ',' or ']' at line: {}, col: {}  (pos: {})", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };
    }
}
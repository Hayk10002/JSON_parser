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

    namespace json_parser::lexer
    {
        class UnexpectedCharacter : public std::exception
        {
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

        class ExpectedADigitOrASign : public UnexpectedCharacter
        {
        public:
            ExpectedADigitOrASign(const Position& pos, char found): UnexpectedCharacter(pos, found, "a digit (from 0 to 9) or a sign (- or +)") {}
        };

        class InvalidLiteral : public std::exception
        {
            std::string m_message;
            Position pos;
            std::string found;
        public:
            InvalidLiteral(const Position& pos, std::string_view found):
                found(found),
                pos(pos),
                m_message(std::format("Invalid literal (\"{}\") at line: {}, col: {}  (pos: {}). Expected \"null\", \"true\" or \"false\".", found, pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedALiteral : public std::exception
        {
            std::string m_message;
            Position pos;
        public:
            ExpectedALiteral(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a literal (\"null\", \"true\" or \"false\") at line: {}, col: {}  (pos: {}).", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };

        class ExpectedANumber : public std::exception
        {
            std::string m_message;
            Position pos;
        public:
            ExpectedANumber(const Position& pos):
                pos(pos),
                m_message(std::format("Expected a number at line: {}, col: {}  (pos: {}).", pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };
    }
}
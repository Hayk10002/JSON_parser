#pragma once

#include <concepts>
#include <exception>
#include <variant>
#include <string>
#include <format>

#include "position.hpp"

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
        ParserError(const ParserError<ETs...>& other)
        {
            std::visit([this](const auto& arg){ inner = arg; }, other);
        }

        template<std::derived_from<std::exception> ...ETs>
        ParserError(ParserError<ETs...>&& other)
        {
            std::visit([this](auto&& arg){ inner = std::move(arg); }, std::move(other));
        }

        template<std::derived_from<std::exception> ET>
        ParserError(ET&& other): inner(std::forward<ET>(other)) {}
    };

    namespace json_parser::lexer
    {
        class UnexpectedCharacter : public std::exception
        {
            std::string m_message;
        public:
            UnexpectedCharacter(const Position& pos, char found, std::string_view expected_text = ""):
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

        class InvalidLiteral : public std::exception
        {
            std::string m_message;
        public:
            InvalidLiteral(const Position& pos, std::string_view found):
                m_message(std::format("Invalid literal (\"{}\") at line: {}, col: {}  (pos: {}). Expected \"null\", \"true\" or \"false\".", found, pos.line, pos.col, pos.pos))
            {}

            virtual const char* what() const noexcept override { return m_message.c_str(); }
        };
    }
}
#pragma once

#include <optional>
#include <variant>
#include <tuple>
#include <concepts>
#include <span>

#include "utils.hpp"
#include "expected.hpp"

namespace hayk10002::parser_types
{
    /// Concept for parsers
    /// Expected from all parser types, that if parsing failes, the input will not be modified (cannot enforce this)
    template<typename T>
    concept ParserType = requires(T t)
    {
        // requires the type to have associated types for input, return and error types
        typename T::InputType;
        typename T::ReturnType;
        typename T::ErrorType;

        // requires the type to have a function named parse, that will take a parameter with type std::span<InputType>, and return a value or error with type itlib::expected<ReturnType, ErrorType>
        requires requires(std::span<typename T::InputType>& input)
        { 
            { t.parse(input) } -> std::same_as<itlib::expected<typename T::ReturnType, typename T::ErrorType>>;
        };
    };

    
    /// @brief A parser, that can run other parsers and if one of them succeds, this returns the successfully parsed value
    /// Expects from all parsers, that in case of an error the input will not be modified
    template<ParserType FirstType, ParserType ...Types>
        // All input types need to be the same type
        requires (std::same_as<typename FirstType::InputType, typename Types::InputType> && ...)
    class Or
    {
    public:
        using InputType = typename FirstType::InputType;

        // In case of success  
        using ReturnType = std::variant<typename FirstType::ReturnType, typename Types::ReturnType...>;

        // In case of error, that means all parsers failed, so returns all the errors
        using ErrorType = std::tuple<typename FirstType::ErrorType, typename Types::ErrorType...>;

        // In case of error, will be empty, in case of success the failed parser errors will be saved
        using InfoType = std::tuple<std::optional<typename FirstType::ErrorType>, std::optional<typename Types::ErrorType>...>;

        Or(FirstType& first_parser, Types&... parsers) : m_parsers{first_parser, parsers...} {}

    private:
        InfoType m_info{};
        std::tuple<FirstType&, Types&...> m_parsers;

    public:
        itlib::expected<ReturnType, ErrorType> parse(std::span<InputType>& input)
        {
            // if a return value is present, hold it
            std::optional<ReturnType> return_val{};

            // for each parser
            for_each_index( 
            [&]<size_t I>()
            {
                // if some other parser already returned a value, do nothing
                if(return_val) return std::monostate{};

                // get the parser
                auto parser = std::get<I>(m_parsers);

                // try to parse
                auto res = parser.parse(input);

                //if successfull
                if (res.has_value())
                {
                    // save the value
                    return_val = ReturnType{std::in_place_index_t<I>{}, std::move(res).value()};
                }
                else
                {
                    // save the error
                    std::get<I>(m_info) = std::move(res).error();
                }

                return std::monostate{};
            }, std::make_integer_sequence<std::size_t, sizeof...(Types) + 1>{});

            // if some parser parsed successfuly
            if(return_val)
            {
                // return the value
                return ReturnType{std::move(*return_val)};
            }
            else
            {
                // move error information from info and return
                InfoType info = std::move(m_info);

                // clear m_info
                m_info = InfoType{};

                return itlib::unexpected(transform([](auto&& elem) { return *elem; }, std::move(info)));
            }
        }

        // Returns additional info about the last parse call
        const InfoType& get_info() { return m_info; }
    };

        /// @brief A parser, that can run other parsers and if one of them succeds, this returns the successfully parsed value
    template<ParserType FirstType, ParserType ...Types>
        // All input types need to be the same type
        requires (std::same_as<typename FirstType::InputType, typename Types::InputType> && ...)
    class Seq
    {
    public:
        using InputType = typename FirstType::InputType;

        // In case of success  
        using ReturnType = std::tuple<typename FirstType::ReturnType, typename Types::ReturnType...>;

        // In case of error, that means some parser failed, returns the error
        using ErrorType = std::variant<typename FirstType::ErrorType, typename Types::ErrorType...>;

        // In case of success, will be empty, in case of error the successfully parsed values will be saved
        using InfoType = std::tuple<std::optional<typename FirstType::ReturnType>, std::optional<typename Types::ReturnType>...>;

        Seq(FirstType& first_parser, Types&... parsers) : m_parsers{first_parser, parsers...} {}

    private:
        InfoType m_info{};
        std::tuple<FirstType&, Types&...> m_parsers;

    public:
        itlib::expected<ReturnType, ErrorType> parse(std::span<InputType>& input)
        {
            // in case of error, we need backup to restore the input to its starting state
            std::span input_backup = input;

            // will hold the error in case of one
            std::optional<ErrorType> return_err{};

            for_each_index( 
            [&]<size_t I>()
            {
                // if some previous parser returned error, do nothing
                if(return_err) return std::monostate{};

                // get the parser
                auto parser = std::get<I>(m_parsers);

                // trye parser to parse
                auto res = parser.parse(input);

                // if successfull
                if (res.has_value())
                {
                    // save the result
                    std::get<I>(m_info) = std::move(res).value();
                }
                else
                {
                    // else restore the input and save the error
                    input = input_backup;
                    return_err = ErrorType{std::in_place_index_t<I>{}, std::move(res).error()};
                }

                return std::monostate{};
            }, std::make_integer_sequence<std::size_t, sizeof...(Types) + 1>{});

            // if got an error return it
            if(return_err)
            {
                return itlib::unexpected(ErrorType{std::move(*return_err)});
            }
            else
            {
                // move parsed values from m_info, and return them
                InfoType info = std::move(m_info);

                // clear m_info
                m_info = InfoType{};
                return transform([](auto&& elem) { return *elem; }, std::move(info));
            }
        }
        
        // Returns additional info about the last parse call
        const InfoType& get_info() { return m_info; }
    };
}
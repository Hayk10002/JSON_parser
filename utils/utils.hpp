#pragma once

#include <tuple>
#include <variant>

namespace hayk10002
{
    /// @brief helper struct to overload lambda functions 
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };


    /// @brief a class that cannot be constructed wihtout wierd ways, effectively does not have valid values 
    struct NonConstructible { NonConstructible() = delete; };

    namespace detail
    {
        template<typename...Ts, typename Function, std::size_t... Is>
        auto transform_impl(Function function, std::index_sequence<Is...>, std::tuple<Ts...>& inputs)
        {
            return std::tuple<std::invoke_result_t<Function, Ts>...>{function(std::get<Is>(inputs))...};
        }

        template<typename...Ts, typename Function, std::size_t... Is>
        auto transform_impl(Function function, std::index_sequence<Is...>, std::tuple<Ts...> const& inputs)
        {
            return std::tuple<std::invoke_result_t<Function, Ts>...>{function(std::get<Is>(inputs))...};
        }

        template<typename...Ts, typename Function, std::size_t... Is>
        auto transform_impl(Function function, std::index_sequence<Is...>, std::tuple<Ts...>&& inputs)
        {
            return std::tuple<std::invoke_result_t<Function, Ts>...>{function(std::get<Is>(std::move(inputs)))...};
        }
    }
    
    // std::transform equivalent for std::tuples

    template<typename... Ts, typename Function>
    auto transform(Function function, std::tuple<Ts...>& inputs)
    {
        return detail::transform_impl(function, std::make_index_sequence<sizeof...(Ts)>{}, inputs);
    }

    template<typename... Ts, typename Function>
    auto transform(Function function, std::tuple<Ts...> const& inputs)
    {
        return detail::transform_impl(function, std::make_index_sequence<sizeof...(Ts)>{}, inputs);
    }

    template<typename... Ts, typename Function>
    auto transform(Function function, std::tuple<Ts...>&& inputs)
    {
        return detail::transform_impl(function, std::make_index_sequence<sizeof...(Ts)>{}, std::move(inputs));
    }

    // calls operator() with templated size_t parameter for each number in index_sequence, almost like compile time for loop, returned values from functions will be stored in a tuple, functions can't return void

    template<typename Function, std::size_t... Is>
    auto for_each_index(Function function, std::index_sequence<Is...>)
    {
        return std::tuple<decltype(function.template operator()<Is>())...>{function.template operator()<Is>()...};
    }

    template<typename FVar, typename SVar>
    struct variant_sum { using type = void; };

    template<typename ...FTypes, typename ...STypes>
    struct variant_sum<std::variant<FTypes...>, std::variant<STypes...>> { using type = std::variant<FTypes..., STypes...>; };

    template<typename FVar, typename SVar>
    using variant_sum_t = typename variant_sum<FVar, SVar>::type;
}
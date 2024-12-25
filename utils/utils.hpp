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

    /// @brief a class that can hold a reference or an owned value
    template <typename T>
        requires requires { !std::is_reference_v<T>; } 
    class RefOrOwned
    {
        std::variant<T, T*> m_inner;
    public:
        RefOrOwned(T& lref) : m_inner{ std::in_place_type<T*>, &lref } {}
        RefOrOwned(const T& clref) : m_inner{ std::in_place_type<T>, clref } {}
        RefOrOwned(T&& rref) : m_inner{ std::in_place_type<T>, std::move(rref) } {}
        
        T& get() 
        { 
            if (std::holds_alternative<T*>(m_inner)) 
                return *std::get<T*>(m_inner);
            return std::get<T>(m_inner);
        }
        
        const T& get() const
        { 
            if (std::holds_alternative<T*>(m_inner)) 
                return *std::get<T*>(m_inner);
            return std::get<T>(m_inner);
        }
    };

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
    struct param_pack_sum { using type = void; };

    template<template <typename ...T> typename ContainerT, typename ...FTypes, typename ...STypes>
    struct param_pack_sum<ContainerT<FTypes...>, ContainerT<STypes...>> { using type = ContainerT<FTypes..., STypes...>; };

    template<typename FVar, typename SVar>
    using param_pack_sum_t = typename param_pack_sum<FVar, SVar>::type;

    template<typename ... newTypes, typename ... oldTypes>
    auto cast_variant(const std::variant<oldTypes...>& var) 
    {
        return std::visit([]<typename T>(T && arg) -> std::variant<newTypes...> 
        {
            if constexpr (std::disjunction_v<std::is_same<std::decay_t<T>, newTypes>...>) return arg;
            else throw std::bad_variant_access();
        }, var);
    }

    template<typename ... newTypes, typename ... oldTypes>
    auto cast_variant(std::variant<oldTypes...>&& var) 
    {
        return std::visit([]<typename T>(T && arg) -> std::variant<newTypes...> 
        {
            if constexpr (std::disjunction_v<std::is_same<std::decay_t<T>, newTypes>...>) return std::move(arg);
            else throw std::bad_variant_access();
        }, var);
    }
}
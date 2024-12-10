#pragma once
#include <variant>
#include <vector>
#include <unordered_map>
#include "utf8_string.hpp"
#include <iterator>

namespace hayk10002
{
    template<typename Json>
    struct json_traits;

    class Json;

    template<>
    struct json_traits<Json>
    {
        using NullType      = std::monostate;
        using BoolType      = bool;
        using IntType       = int64_t;
        using FloatType     = double;
        using StringType    = UTF8string;
        using ArrayType     = std::vector<Json>;
        using ObjectType    = std::unordered_map<StringType, Json>; 
    };
}
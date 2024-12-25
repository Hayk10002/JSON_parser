#pragma once

#include <iostream>
#include <iomanip>
#include <variant>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

#include "utils.hpp"
#include "utf8_string.hpp"
namespace hayk10002
{   


    class Json
    {
    public:
        using NullType      = std::monostate;
        using BoolType      = bool;
        using IntType       = int64_t;
        using FloatType     = double;
        using StringType    = UTF8string;
        using ArrayType     = std::vector<Json>;
        using ObjectType    = std::unordered_map<StringType, Json>;

    private:
        
        std::variant<
            NullType,
            BoolType,
            IntType,
            FloatType,
            StringType,
            ArrayType,
            ObjectType
        > m_data;

    public:

        static Json null         ()                      { return Json{}; }
        
        static Json boolean      (const BoolType&   val) { Json res; res.m_data = val; return res; }
        static Json number_int   (const IntType&    val) { Json res; res.m_data = val; return res; }
        static Json number_float (const FloatType&  val) { Json res; res.m_data = val; return res; }
        static Json string       (const StringType& val) { Json res; res.m_data = val; return res; }
        static Json array        (const ArrayType&  val) { Json res; res.m_data = val; return res; }
        static Json object       (const ObjectType& val) { Json res; res.m_data = val; return res; }

        static Json boolean      (BoolType&&   val) { Json res; res.m_data = std::move(val); return res; }
        static Json number_int   (IntType&&    val) { Json res; res.m_data = std::move(val); return res; }
        static Json number_float (FloatType&&  val) { Json res; res.m_data = std::move(val); return res; }
        static Json string       (StringType&& val) { Json res; res.m_data = std::move(val); return res; }
        static Json array        (ArrayType&&  val) { Json res; res.m_data = std::move(val); return res; }
        static Json object       (ObjectType&& val) { Json res; res.m_data = std::move(val); return res; }

        bool is_null         () const { return std::holds_alternative<NullType>  (m_data); }
        bool is_boolean      () const { return std::holds_alternative<BoolType>  (m_data); }
        bool is_number_int   () const { return std::holds_alternative<IntType>   (m_data); }
        bool is_number_float () const { return std::holds_alternative<FloatType> (m_data); }
        bool is_string       () const { return std::holds_alternative<StringType>(m_data); }
        bool is_array        () const { return std::holds_alternative<ArrayType> (m_data); }
        bool is_object       () const { return std::holds_alternative<ObjectType>(m_data); }

        NullType&   get_null         () { return std::get<NullType>  (m_data); }
        BoolType&   get_boolean      () { return std::get<BoolType>  (m_data); }
        IntType&    get_number_int   () { return std::get<IntType>   (m_data); }
        FloatType&  get_number_float () { return std::get<FloatType> (m_data); }
        StringType& get_string       () { return std::get<StringType>(m_data); }
        ArrayType&  get_array        () { return std::get<ArrayType> (m_data); }
        ObjectType& get_object       () { return std::get<ObjectType>(m_data); }

        const NullType&   get_null         () const { return std::get<NullType>  (m_data); }
        const BoolType&   get_boolean      () const { return std::get<BoolType>  (m_data); }
        const IntType&    get_number_int   () const { return std::get<IntType>   (m_data); }
        const FloatType&  get_number_float () const { return std::get<FloatType> (m_data); }
        const StringType& get_string       () const { return std::get<StringType>(m_data); }
        const ArrayType&  get_array        () const { return std::get<ArrayType> (m_data); }
        const ObjectType& get_object       () const { return std::get<ObjectType>(m_data); }
        

        friend std::ostream& operator<<(std::ostream& out, const Json& val)
        {
            std::visit(overloaded{
                [&out](const NullType&      arg) { std::cout << "null"; },
                [&out](const BoolType&      arg) { out << (arg ? "true" : "false"); },
                [&out](const IntType&       arg) { out << arg; },
                [&out](const FloatType&     arg) { out << arg; },
                [&out](const StringType&    arg) { out << "\"" << arg << "\""; },
                [&out](const ArrayType&     arg) 
                { 
                    out << '[';
                    for (int i = 0; i < arg.size(); i++) out << (i != 0 ? ", " : "") << arg[i];
                    out << ']';
                },
                [&out](const ObjectType&    arg) 
                {
                    out << '{';
                    bool first = true;
                    for (const auto& [key, value] : arg) 
                    {
                        if (!first) out << ", ";
                        out << "\"" << key << "\"" << ": " << value;
                        first = false;
                    }
                    out << '}';
                }
            }, val.m_data);
            return out;
        }

        /// @note this can return false results due to floating point presicion
        auto operator<=>(const Json& other) const = default;
    };
}
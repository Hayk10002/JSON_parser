#include <iostream>
#include <iomanip>
#include <variant>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace hayk10002
{   
    //helper struct to overload lambda functions 
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };


    class Json
    {
    public:
        using NullType      = std::monostate;
        using BoolType      = bool;
        using IntType       = int64_t;
        using FloatType     = double;
        using StringType    = std::string;

        template<typename T = Json>
        using ArrayTypeT    = std::vector<T>;
        using ArrayType     = ArrayTypeT<>;

        template<typename T = Json>
        using ObjectTypeT   = std::unordered_map<StringType, T>; 
        using ObjectType    = ObjectTypeT<>;

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

        static Json null        ()                       { return Json{}; }
        static Json boolean     (const BoolType&    val) { Json res; res.m_data = val; return res; }
        static Json number_int  (const IntType&     val) { Json res; res.m_data = val; return res; }
        static Json number_float(const FloatType&   val) { Json res; res.m_data = val; return res; }
        static Json string      (const StringType&  val) { Json res; res.m_data = val; return res; }
        static Json array       (const ArrayType&   val) { Json res; res.m_data = val; return res; }
        static Json object      (const ObjectType&  val) { Json res; res.m_data = val; return res; }

        friend std::ostream& operator<<(std::ostream& out, const Json& val)
        {
            std::visit(overloaded{
                [&out](const NullType&      arg) { std::cout << "null"; },
                [&out](const BoolType&      arg) { out << (arg ? "true" : "false"); },
                [&out](const IntType&       arg) { out << arg; },
                [&out](const FloatType&     arg) { out << arg; },
                [&out](const StringType&    arg) { out << std::quoted(arg); },
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
                        out << std::quoted(key) << ": " << value;
                        first = false;
                    }
                    out << '}';
                }
            }, val.m_data);
            return out;
        }
    };
}
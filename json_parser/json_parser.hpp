#include <iostream>
#include <variant>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

namespace hayk10002
{    
    class Json
    {
    public:
        using NullType = std::monostate;
        using BoolType = bool;
        using IntType = int64_t;
        using UIntType = uint64_t;
        using FloatType = double;
        using StringType = std::string;
        template<typename T = Json>
        using ArrayTypeT = std::vector<T>;
        using ArrayType = ArrayTypeT<>;
        template<typename T = Json>
        using ObjectTypeT = std::unordered_map<StringType, T>; 
        using ObjectType = ObjectTypeT<>;

    private:
        
        std::variant<
            NullType,
            BoolType,
            IntType,
            UIntType,
            FloatType,
            StringType,
            ArrayType,
            ObjectType
        > m_data;

    public:

        static Json null() { return Json{}; }
        static Json boolean(const BoolType& val) { Json res; res.m_data = val; return res; }
        static Json number(const IntType& val) { Json res; res.m_data = val; return res; }
        static Json number(const UIntType& val) { Json res; res.m_data = val; return res; }
        static Json number(const FloatType& val) { Json res; res.m_data = val; return res; }
        static Json string(const StringType& val) { Json res; res.m_data = val; return res; }
        static Json array(const ArrayType& val) { Json res; res.m_data = val; return res; }
        static Json object(const ObjectType& val) { Json res; res.m_data = val; return res; }

        friend std::ostream& operator<<(std::ostream& out, const Json& val)
        {
            
        }
    };
}
#include <iostream>
#include <variant>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace hayk10002
{
    class Json;

    typedef std::monostate NullType;
    typedef bool BoolType;
    typedef int64_t IntType;
    typedef uint64_t UIntType;
    typedef double FloatType;
    typedef std::string StringType;
    typedef std::vector<Json> ArrayType;
    typedef std::unordered_map<StringType, Json> ObjectType;

    class Json
    {
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
        Json() = default;
        Json(BoolType val): m_data(val) {}
        Json(IntType val): m_data(val) {}
        Json(UIntType val): m_data(val) {}
        Json(FloatType val): m_data(val) {}
        Json(StringType val): m_data(val) {}
        Json(ArrayType val): m_data(val) {}
        Json(ObjectType val): m_data(val) {}
    };
}
#include <iostream>
#include <variant>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace hayk10002
{
    typedef std::monostate NullType;

    class Json
    {
    private:
        
        std::variant<
            NullType,
            bool, 
            int64_t, 
            uint64_t, 
            double, 
            std::string, 
            std::vector<Json>, 
            std::unordered_map<std::string, Json>
        > m_data;

    };
}
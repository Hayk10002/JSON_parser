#include "utils.hpp"

#include <concepts>
#include <variant>

int main() {

    static_assert(std::same_as<std::variant<int, float, double>, hayk10002::variant_sum_t<std::variant<int>, std::variant<float, double>>>);
    
    return 0;
}

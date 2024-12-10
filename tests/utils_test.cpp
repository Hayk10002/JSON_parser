#include "utils.hpp"

#include <concepts>
#include <variant>
#include <tuple>

int main() {

    static_assert(std::same_as<std::variant<int, float, double>, hayk10002::param_pack_sum_t<std::variant<int>, std::variant<float, double>>>);
    static_assert(std::same_as<std::tuple<int, float, double>, hayk10002::param_pack_sum_t<std::tuple<int>, std::tuple<float, double>>>);
    
    return 0;
}

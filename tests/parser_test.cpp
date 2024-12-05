#include <iostream>
#include <string>

#include "parser.hpp"

using hayk10002::parser_types::Or;
using hayk10002::parser_types::Seq;

struct TrueParser
{
    using InputType = char;
    using ReturnType = bool;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(std::span<char>& input)
    {
        if(input.size() < 4 || std::string_view{input.begin(), input.begin() + 4} != "true") return itlib::unexpected("Expected \"true\"");
        input = input.subspan(4);
        return true;
    }
};

struct FalseParser
{
    using InputType = char;
    using ReturnType = bool;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(std::span<char>& input)
    {
        if(input.size() < 5 || std::string_view{input.begin(), input.begin() + 5} != "false") return itlib::unexpected("Expected \"false\"");
        input = input.subspan(5);
        return false;
    }
};

struct BoolParser
{
    using InputType = char;
    using ReturnType = bool;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(std::span<char>& input)
    {
        TrueParser tp{};
        FalseParser fp{};

        Or<TrueParser, FalseParser> p{tp, fp};

        auto res = p.parse(input);
        if (res.has_error()) return itlib::unexpected("Expected boolean value (\"true\" or \"false\")");
        return res.value().index() == 0;
    }
};

struct TwoBoolsParser
{
    using InputType = char;
    using ReturnType = std::array<bool, 2>;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(std::span<char>& input)
    {
        BoolParser bp{};

        Seq<BoolParser, BoolParser> p{bp, bp};

        auto res = p.parse(input);
        if (res.has_error()) return itlib::unexpected("Expected boolean value (\"true\" or \"false\")");
        return ReturnType{std::get<0>(res.value()), std::get<1>(res.value())};
    }
};



int main() {

    TwoBoolsParser parser{};
    std::string input = "truetfalse";
    std::span input_span = input;
    auto res = parser.parse(input_span);
    if (res.has_value()) std::cout << "value" << res.value()[0] << res.value()[1];
    else std::cout << "error " << res.error();
    return 0;
}

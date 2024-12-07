#include <iostream>
#include <string>

#include "parser.hpp"
#include <vector>

using hayk10002::parser_types::Or;
using hayk10002::parser_types::Seq;
using hayk10002::parser_types::Nothing;
using hayk10002::parser_types::Cycle;

struct SpanWrapper
{
    std::span<char> span;
    std::span<char> get_pos() { return span; }
    void set_pos(std::span<char> span) { this->span = span; }

    SpanWrapper(std::span<char> sp): span{sp} {}
    operator std::span<char>() { return span; }
};

struct TrueParser
{
    using InputType = SpanWrapper;
    using ReturnType = bool;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(SpanWrapper& input)
    {
        if(input.span.size() < 4 || std::string_view{input.span.begin(), input.span.begin() + 4} != "true") return itlib::unexpected("Expected \"true\"");
        input = input.span.subspan(4);
        return true;
    }
};

struct FalseParser
{
    using InputType = SpanWrapper;
    using ReturnType = bool;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(SpanWrapper& input)
    {
        if(input.span.size() < 5 || std::string_view{input.span.begin(), input.span.begin() + 5} != "false") return itlib::unexpected("Expected \"false\"");
        input = input.span.subspan(5);
        return false;
    }
};

struct BoolParser
{
    using InputType = SpanWrapper;
    using ReturnType = bool;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(SpanWrapper& input)
    {
        TrueParser tp{};
        FalseParser fp{};

        Or p{tp, fp};

        auto res = p.parse(input);
        if (res.has_error()) return itlib::unexpected("Expected boolean value (\"true\" or \"false\")");
        return res.value().index() == 0;
    }
};

struct TwoOrThreeBoolsParser
{
    using InputType = SpanWrapper;
    using ReturnType = std::vector<bool>;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(SpanWrapper& input)
    {
        BoolParser bp{};
        Nothing<SpanWrapper> np{};
        Or b_or_np{bp, np};
        Seq p{bp, bp, b_or_np};

        auto res = p.parse(input);
        if (res.has_error()) return itlib::unexpected("Expected boolean value (\"true\" or \"false\")");
        ReturnType val{std::get<0>(res.value()), std::get<1>(res.value())};
        auto third = std::get<2>(res.value());
        if (third.index() == 0) val.push_back(std::get<0>(third));
        return val;
    }
};

struct GreedyEveryOtherBoolParser
{
    using InputType = SpanWrapper;
    using ReturnType = std::vector<bool>;
    using ErrorType = std::string;

    itlib::expected<ReturnType, ErrorType> parse(SpanWrapper& input)
    {
        BoolParser bp{};

        // parses one bool value as a value, and one as a separator, so "truefalsetrue" will be parsed to [1, 1] (the middle "false" was regarded as a separator and value thrown away)
        Cycle p{bp, bp};

        auto res = p.parse(input);
        // res can't be an error
        return std::move(res).value();
    }
};



int main() {

    TwoOrThreeBoolsParser parser{};

    for(int i = 0; i < 13; i++){
        std::string input;
        if (i < 4) input = std::string(i / 2 ? "true" : "false") + (i % 2 ? "true" : "false");
        else input = std::string((i - 4) / 4 ? "true" : "false") + ((i - 4) / 2 % 2 ? "true" : "false") + ((i - 4) % 2 ? "true" : "false");
        SpanWrapper input_span{input};
        auto res = parser.parse(input_span);
        if (res.has_value())
        {
            std::cout << "value";
            for(bool val: res.value()) std::cout << val;
            std::cout << std::endl;
        }
        else std::cout << "error " << res.error() << std::endl;
    }   

    GreedyEveryOtherBoolParser parser2{};

    srand(time(0));
    int n = rand() % 10;
    std::cout << n << ' ';
    std::string input{};
    for (int i = 0; i < n; i++) 
    {
        bool val = rand() % 2;
        std::cout << val;
        input += (val ? "true" : "false");
    }
    std::cout << std::endl << input << std::endl;
    SpanWrapper input_span{input};
    auto res = parser2.parse(input_span);
    std::cout << "value";
    for(bool val: res.value()) std::cout << val;
    std::cout << std::endl;
    return 0;
}

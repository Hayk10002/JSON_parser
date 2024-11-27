#include <iostream>

#include "json.hpp"

using namespace hayk10002;

int main()
{
    // test value taken from https://github.com/briandfoy/json-acceptance-tests/blob/master/json-checker/pass1.json 
    Json value = Json::array(
    {
        Json::string("JSON Test Pattern pass1"),
        Json::object({{"object with 1 member", Json::array({Json::string("array with 1 element")})}}),
        Json::object({}),
        Json::array({}),
        Json::number_int(-42),
        Json::boolean(true),
        Json::boolean(false),
        Json::null(),
        Json::object(
        {
            {"integer", Json::number_int(1234567890)},
            {"real", Json::number_float(-9876.543210)},
            {"e", Json::number_float(0.123456789e-12)},
            {"E", Json::number_float(1.234567890E+34)},
            {"", Json::number_float(23456789012E66)},
            {"zero", Json::number_int(0)},
            {"one", Json::number_int(1)},
            {"space", Json::string(" ")},
            {"quote", Json::string("")},
            {"backslash", Json::string("\\")},
            {"controls", Json::string("\b\f\n\r\t")},
            {"slash", Json::string("/ & /")},
            {"alpha", Json::string("abcdefghijklmnopqrstuvwyz")},
            {"ALPHA", Json::string("ABCDEFGHIJKLMNOPQRSTUVWYZ")},
            {"digit", Json::string("0123456789")},
            {"0123456789", Json::string("digit")},
            {"special", Json::string("`1~!@#$%^&*()_+-={':[,]}|;.</>?")},
            {"hex", Json::string("\u0123\u4567\u89AB\uCDEF\uabcd\uef4A")},
            {"true", Json::boolean(true)},
            {"false", Json::boolean(false)},
            {"null", Json::null()},
            {"array", Json::array({})},
            {"object", Json::object({})},
            {"address", Json::string("50 St. James Street")},
            {"url", Json::string("http://www.JSON.org/")},
            {"comment", Json::string("// /* <!-- --")},
            {"# -- --> */", Json::string(" ")},
            {" s p a c e d ", Json::array({Json::number_int(1), Json::number_int(2), Json::number_int(3), Json::number_int(4), Json::number_int(5), Json::number_int(6), Json::number_int(7)})},
            {"compact", Json::array({Json::number_int(1), Json::number_int(2), Json::number_int(3), Json::number_int(4), Json::number_int(5), Json::number_int(6), Json::number_int(7)})},
            {"jsontext", Json::string("{\"object with 1 member\":[\"array with 1 element\"]}")},
            {"quotes", Json::string("&#34; \u0022 %22 0x22 034 &#x22;")},
            {"/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?", Json::string("A key can be any string")}            
        }),
        Json::number_float(0.5),
        Json::number_float(98.6),
        Json::number_float(99.44),
        Json::number_int(1066),
        Json::number_float(1e1),
        Json::number_float(0.1e1),
        Json::number_float(1e-1),
        Json::number_float(1e00),
        Json::number_float(2e+00),
        Json::number_float(2e-00),
        Json::string("rosebud")
    });

    std::cout << value;

    return 0;
}

/*
this test value taken from https://github.com/briandfoy/json-acceptance-tests/blob/master/json-checker/pass1.json 
[
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&*()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /* <!-- --",
        "# -- --> * /": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"]

*/
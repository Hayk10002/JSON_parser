# JSON Parser

This project is a JSON parser written in C++ with CMake. It provides functionality to parse JSON strings into C++ objects and handle various JSON data types.

## Features

- **Parse JSON Strings**: Convert JSON strings into C++ objects.
- **Support for JSON Data Types**: Handle JSON objects, arrays, strings, numbers, booleans, and null values.
- **Error Handling**: Provides detailed error messages for invalid JSON input.

## Usage

### Parsing a JSON String

To parse a JSON string, use the `JsonParser` class. Here is an example:

```cpp
#include <iostream>
#include "json.hpp"
#include "json_parser.hpp"

using namespace hayk10002;

int main() {
    std::string json_string = R"({
        "name": "John Doe",
        "age": 30,
        "is_student": false,
        "courses": ["Math", "Science"],
        "address": {
            "street": "123 Main St",
            "city": "Anytown"
        }
    })";

    json_parser::JsonParser parser;
    json_parser::lexer::Cursor input{json_string};
    auto result = parser.parse(input);

    if (result.has_error()) {
        std::cout << "Error: " << result.error().what() << std::endl;
    } else {
        std::cout << "Parsed JSON: " << result.value() << std::endl;
    }

    return 0;
}


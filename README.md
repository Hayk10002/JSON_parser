# JSON Parser

This project is a JSON parser written in C++ with CMake. It provides functionality to parse JSON strings into C++ objects and handle various JSON data types.

## Features

- **Parse JSON Strings**: Convert JSON strings into C++ objects.
- **Support for JSON Data Types**: Handle JSON objects, arrays, strings, numbers, booleans, and null values.
- **Error Handling**: Provides detailed error messages for invalid JSON input.

## Usage

Copy files from the folder **json_parser** into your project's directory and include them to your files.
Or if you use cmake, 

1. Clone the JSON parser repository into your project's directory:

    ```
    git clone https://github.com/Hayk10002/json_parser.git
    ```

1. Modify your `CMakeLists.txt` to include the JSON parser:

    ```
    cmake_minimum_required(VERSION 3.18)
    project(YourProjectName)

    # Add the JSON parser
    add_subdirectory(json_parser)

    # Link the JSON parser library to your target
    add_executable(YourExecutable main.cpp)
    target_link_libraries(YourExecutable PRIVATE json_parser)
    ```

1. And then include the JSON parser headers in your source files:

    ```
    #include "json.hpp"
    #include "json_parser.hpp"
    ```


## Parsing a JSON String

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
```

## Notes

This library treats the hex escape sequences (\uhhhh) as utf-16 encoded, so if the encoding is not correct, InvalidEncoding error will be returned. For example the string \uD83D\uDE00 in json string will be read as 😀.




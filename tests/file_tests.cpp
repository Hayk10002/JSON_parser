#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#include "json_parser.hpp"

#ifndef TEST_DIRECTORY_PATH
#define TEST_DIRECTORY_PATH "/default/path"
#endif

// Define a test function that processes a string (file content).
void test(std::string_view content)
{
    using namespace hayk10002;

    json_parser::JsonParser parser;
    json_parser::lexer::Cursor input{content};
    auto result = parser.parse(input);

    if (result.has_error()) std::cout << result.error().what() << std::endl;
    else std::cout << result.value() << std::endl;
}

void testFolder(std::string_view folder_name)
{
    namespace fs = std::filesystem;

    // Directory path to process
    fs::path directoryPath = fs::path(TEST_DIRECTORY_PATH) / "file_tests" / folder_name; // Change to your directory

    try {
        // Check if the directory exists
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Error: Directory does not exist or is not a directory." << std::endl;
            return;
        }

        // Iterate over the files in the directory
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                std::ifstream file(entry.path());

                if (!file) {
                    std::cerr << "Error: Could not open file " << entry.path() << std::endl;
                    continue;
                }

                // Read the file content into a string
                std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

                // Call the test function with the file content and name
                std::cout << "Testing file: " << entry.path().filename() << std::endl;
                test(fileContent);
                std::cout << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return;
    }
}

int main() {

    testFolder("fails");    
    testFolder("successes");    
    // testFolder("big_files");    

    return 0;
}

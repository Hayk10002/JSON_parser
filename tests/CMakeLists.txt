project(tests)

add_executable(main_test main.cpp)
target_link_libraries(main_test PRIVATE json_parser)

add_test(NAME MainTest COMMAND main_test.exe)


# tests for each standalone header file

# Define a function to handle the process
function(generate_test_for_header header_file target_name)
    # Get the base name of the header file without extension
    get_filename_component(name ${header_file} NAME_WE) # NAME without .hpp

    # Generate the test .cpp file name
    set(test_cpp_file ${CMAKE_CURRENT_SOURCE_DIR}/${name}_test.cpp)

    # Create the test .cpp file with #include directive if it was not created
    if(NOT EXISTS ${test_cpp_file})
        file(WRITE ${test_cpp_file} "#include \"${header_file}\"\n\nint main() {\n    return 0;\n}\n")
    endif()

    # Add executable target for the test
    add_executable(${name}_test ${test_cpp_file})
    target_link_libraries(${name}_test PRIVATE ${target_name})

    # Add a test to CTest
    add_test(NAME ${name}Test COMMAND ${name}_test)
endfunction()

# Define a list of header files
set(HEADER_FILES
    parser.hpp
    json_parser.hpp
    parser_error.hpp
    position.hpp
    expected.hpp
    utils.hpp
)

# The targets of each header file defined above
set(TARGET_NAMES
    json_parser
    json_parser
    json_parser
    json_parser
    utils
    utils
)

# Call the function for each file in the list
foreach(header target IN ZIP_LISTS HEADER_FILES TARGET_NAMES)
    generate_test_for_header(${header} ${target})
endforeach()

target_compile_options(parser_test PRIVATE "-fconcepts-diagnostics-depth=4")
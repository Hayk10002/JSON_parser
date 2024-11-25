project(tests)

add_executable(main_test main.cpp)
target_link_libraries(main_test PRIVATE json_parser)

add_test(NAME MainTest COMMAND main_test.exe)
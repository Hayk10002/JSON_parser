cmake_minimum_required(VERSION 3.5.0)
project(JSON_parser VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(json_parser)
add_subdirectory(tests)

option(ENABLE_TESTING ON)
option(ENABLE_CPACK OFF)

if(ENABLE_TESTING)
    include(CTest)
    enable_testing()

    add_test(NAME MainTest COMMAND tests/main_test.exe)
endif()


if(ENABLE_CPACK)
    set(CPACK_PROJECT_NAME ${PROJECT_NAME})
    set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
    include(CPack)
endif()

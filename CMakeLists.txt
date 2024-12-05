cmake_minimum_required(VERSION 3.5.0)
project(JSON_parser VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(ENABLE_TESTING "enable testing" ON)
option(ENABLE_CPACK "enable cpack" OFF)

if(ENABLE_TESTING)
    include(CTest)
    enable_testing()
endif()

add_subdirectory(utils)
add_subdirectory(utf8_string)
add_subdirectory(json_parser)
add_subdirectory(tests)

if(ENABLE_CPACK)
    set(CPACK_PROJECT_NAME ${PROJECT_NAME})
    set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
    include(CPack)
endif()

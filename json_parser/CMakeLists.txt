set(JSON_parserTargetName json_parser)

project(${JSON_parserTargetName})

add_library(${JSON_parserTargetName} INTERFACE)

target_include_directories(${JSON_parserTargetName}
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
)


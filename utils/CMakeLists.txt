set(UtilsTargetName utils)

project(${UtilsTargetName})

add_library(${UtilsTargetName} INTERFACE)


target_include_directories(${UtilsTargetName}
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
)


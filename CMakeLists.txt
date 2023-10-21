cmake_minimum_required(VERSION 3.15)
project(arrowhead-core-c C)

if (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    if (NOT CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options(-ffile-prefix-map=${PROJECT_SOURCE_DIR}/=)
    endif ()
    if (CMAKE_C_COMPILER_ID MATCHES "GNU")
        add_compile_options(-Wno-missing-field-initializers)
    endif ()
endif ()

add_subdirectory(libraries)

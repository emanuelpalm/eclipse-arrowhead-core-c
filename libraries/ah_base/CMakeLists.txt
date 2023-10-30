cmake_minimum_required(VERSION 3.15)
project(ah_base LANGUAGES C VERSION 1.0.0)

set(CMAKE_C_STANDARD 99)

set(
    AH_BASE_SOURCE_FILES
    include/ah/alloc.h
    include/ah/bit.h
    include/ah/buf.h
    include/ah/ckdint.h
    include/ah/def.h
    include/ah/err.h
    include/ah/time.h
    internal/ahi/alloc.h
    internal/ahi/bit.h
    internal/ahi/build.gen.in
    internal/ahi/ckdint.h
    internal/ahi/def.h
    src/alloc/align.c
    src/alloc/bump.c
    src/alloc/slab.c
)
add_library(ah_base STATIC ${AH_BASE_SOURCE_FILES})

target_include_directories(
    ah_base
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/internal>
)

add_subdirectory(platform)
if (NOT TARGET ah_base_platform)
    message(FATAL_ERROR "No 'ah_base_platform' target exists for '${CMAKE_SYSTEM_NAME}'.")
endif ()
target_link_libraries(ah_base PUBLIC ah_base_platform)

include(FindGit)
if (GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE AH_BASE_GIT_RESULT
        OUTPUT_VARIABLE AH_BASE_GIT_COMMIT_HASH
        ERROR_VARIABLE AH_BASE_GIT_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )
    if (NOT ${AH_BASE_GIT_RESULT} EQUAL 0)
        message(WARNING "Failed to execute `${GIT_EXECUTABLE} rev-parse --short HEAD`; ${AH_BASE_GIT_ERROR}")
    endif ()
endif ()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/ahi/build.gen.in
    ${CMAKE_CURRENT_SOURCE_DIR}/internal/ahi/build.gen
    @ONLY
)

set(
    AH_BASE_TEST_FILES
    tests/main.c
    tests/suite-alloc-align.c
    tests/suite-alloc-bump.c
    tests/suite-alloc-page.c
    tests/suite-alloc-slab.c
    tests/suite-buf.c
    tests/suite-bufc.c
    tests/suite-ckdint.c
    tests/suite-def.c
    tests/suite-err.c
    tests/suite-time.c
)

add_executable(ah_base_tests ${AH_BASE_TEST_FILES})
target_link_libraries(ah_base_tests PRIVATE ah_base ah_unit)

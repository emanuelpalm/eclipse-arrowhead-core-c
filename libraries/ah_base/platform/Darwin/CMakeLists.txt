cmake_minimum_required(VERSION 3.15)
project(ah_base_platform LANGUAGES C VERSION 1.0.0)

set(CMAKE_C_STANDARD 99)

set(
    AH_BASE_PLATFORM_SOURCE_FILES
    include/ahp/alloc.h
    include/ahp/buf.h
    include/ahp/def.h
    include/ahp/err.h
    include/ahp/loop.h
    include/ahp/time.h
    src/alloc/page.c
    src/err.c
    src/time.c
)

add_library(ah_base_platform STATIC ${AH_BASE_PLATFORM_SOURCE_FILES})

target_include_directories(ah_base_platform PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

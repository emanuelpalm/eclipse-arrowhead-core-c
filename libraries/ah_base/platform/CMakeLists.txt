cmake_minimum_required(VERSION 3.15)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME})
    add_subdirectory(${CMAKE_SYSTEM_NAME})
else()
    message(WARNING "Noes!")
endif ()

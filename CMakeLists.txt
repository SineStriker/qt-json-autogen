cmake_minimum_required(VERSION 3.17)

project(qas_tool)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

set(PROJECT_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)

include(${PROJECT_MODULES_DIR}/All.cmake)

add_subdirectory(src)

add_subdirectory(tools)
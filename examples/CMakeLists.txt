cmake_minimum_required(VERSION 3.17)

project(examples)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

find_package(qastool REQUIRED)

include_directories(${QASTOOL_INCLUDE_DIRS})

# Add modules
include(../scripts/cmake/All.cmake)

# Add includes

add_subdirectory(test1)

add_subdirectory(test2)
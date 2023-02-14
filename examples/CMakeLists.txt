cmake_minimum_required(VERSION 3.17)

project(examples)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

# Add modules
include(../scripts/cmake/All.cmake)

include(../QasUtils.cmake)

# Add includes
include_directories(../include)

add_subdirectory(test1)
cmake_minimum_required(VERSION 3.17)

project(examples)

# Add modules
include(../scripts/cmake/All.cmake)

include(../QasUtils.cmake)

# Add includes
include_directories(../include)

add_subdirectory(test1)
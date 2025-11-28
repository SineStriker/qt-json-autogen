cmake_minimum_required(VERSION 3.17)

project(examples)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif ()

if (QAS_ROOT_PROJECT)
    include(../QasToolMacros.cmake)
    set(QASTOOL_QASC_EXECUTABLE "$<TARGET_FILE:qasc>")
    set(QASTOOL_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../include")
else ()
    find_package(QasTool REQUIRED)
endif ()

include_directories(${QASTOOL_INCLUDE_DIRS})

# Add modules
include(../scripts/cmake/All.cmake)

# Add includes
add_subdirectory(test4)

add_subdirectory(test5)

add_subdirectory(fakeheader)

add_subdirectory(constraint_test)
project(qasc VERSION 0.0.1.1 LANGUAGES CXX)

include(${PROJECT_MODULES_DIR}/All.cmake)

# ----------------------------------
# Add modules
# ----------------------------------
add_qt_module(_qt_libs Core)
add_qt_private_inc(_qt_private_incs Core)

# ----------------------------------
# Add target
# ----------------------------------)
include_directories(../../include)
add_files(_src CURRENT_RECURSE PATTERNS *.h *.c *.cpp)
add_executable(${PROJECT_NAME} ${_src})

# ----------------------------------
# Target...
# ----------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE APP_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${_qt_libs} mocparser)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${_qt_private_incs})

include(../../QasUtils.cmake)
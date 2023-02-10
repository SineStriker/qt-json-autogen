project(qasjson VERSION 0.0.1.1 LANGUAGES CXX)

# ----------------------------------
# List utils
# ----------------------------------
macro(list_add_flatly _list)
    set(_temp_list)
    file(GLOB _temp_list ${ARGN})
    list(APPEND ${_list} ${_temp_list})
    unset(_temp_list)
endmacro()

macro(list_add_recursively _list)
    set(_temp_list)
    file(GLOB_RECURSE _temp_list ${ARGN})
    list(APPEND ${_list} ${_temp_list})
    unset(_temp_list)
endmacro()

# ----------------------------------
# Find libraries
# ----------------------------------
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

list(APPEND _qt_libs Qt${QT_VERSION_MAJOR}::Core)

list(APPEND _qt_private_incs ${Qt${QT_VERSION_MAJOR}Core_PRIVATE_INCLUDE_DIRS})

# ----------------------------------
# Add target
# ----------------------------------
list_add_flatly(_src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE APP_VERSION="${PROJECT_VERSION}")

target_link_libraries(${PROJECT_NAME} PRIVATE ${_qt_libs})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${_qt_private_incs})
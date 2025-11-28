project(constraint_test)

# ----------------------------------
# Add modules
# ----------------------------------
add_qt_module(_qt_libs Core)
add_qt_private_inc(_qt_private_incs Core)

# ----------------------------------
# Add target
# ----------------------------------
add_files(_src CURRENT_RECURSE PATTERNS *.h *.c *.cpp)
add_executable(${PROJECT_NAME} ${_src})

# ----------------------------------
# Target...
# ----------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE ${_qt_libs})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${_qt_private_incs})

if(TRUE)
    set(_headers ${_src})
    list(FILTER _headers INCLUDE REGEX ".*\\.(h|hpp)")
    qas_wrap_cpp(_qasc_src ${_headers} TARGET ${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE ${_qasc_src})
endif()
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
add_files(_src CURRENT_RECURSE PATTERNS *.h *.c *.cpp)
add_executable(${PROJECT_NAME} ${_src})

# ----------------------------------
# Target...
# ----------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE APP_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${_qt_libs} mocparser)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${_qt_private_incs})

if(WIN32)
    # configure rc
    set(WIN32_EXPORT_NAME ${PROJECT_NAME})
    set(WIN32_COPYRIGHT_START_YEAR "2023")
    set(WIN32_COPYRIGHT_END_YEAR "2023")
    set(WIN32_AUTHOR_NAME "Sine Striker")
    set(WIN32_FILE_DESC "Qt Auto Serialization Compiler")
    set(WIN32_PRODUCT_NAME "Qt Tools")
    configure_file(
        res.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/res.rc
        @ONLY
    )
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/res.rc)
endif()
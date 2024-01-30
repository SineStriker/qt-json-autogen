project(core)

# ----------------------------------
# Add target
# ----------------------------------)
add_library(${PROJECT_NAME} INTERFACE)
add_library(${QAS_INSTALL_NAME_LOWER}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# ----------------------------------
# Target...
# ----------------------------------
target_include_directories(${PROJECT_NAME} INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QAS_INSTALL_NAME_LOWER}>"
)

add_qt_module(_qt_libs Core)
target_link_libraries(${PROJECT_NAME} INTERFACE ${_qt_libs})
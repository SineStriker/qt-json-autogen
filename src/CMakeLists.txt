project(qasstream)

# ----------------------------------
# Add target
# ----------------------------------)
add_library(${PROJECT_NAME} INTERFACE)

# ----------------------------------
# Target...
# ----------------------------------
target_include_directories(${PROJECT_NAME} INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QAS_INSTALL_NAME_LOWER}>"
)
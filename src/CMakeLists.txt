project(qasstream)

# ----------------------------------
# Add modules
# ----------------------------------
add_qt_module(_qt_libs Core)
add_qt_private_inc(_qt_private_incs Core)

# ----------------------------------
# Add target
# ----------------------------------)
add_files(_src CURRENT_RECURSE PATTERNS *.h *.c *.cpp DIRECTORIES ../include)
add_library(${PROJECT_NAME} STATIC ${_src})

# ----------------------------------
# Target...
# ----------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE ${_qt_libs})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../include ${_qt_private_incs})

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QAS_INSTALL_NAME_LOWER}>"
)
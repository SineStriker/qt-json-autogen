cmake_minimum_required(VERSION 3.17)

project(qastool VERSION 0.0.2.2)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ----------------------------------
# Configure Options
# ----------------------------------
option(QAS_BUILD_EXAMPLES "Build examples" ON)
option(QAS_BUILD_MOC_EXE "Build Qt moc executable" ON)

# ----------------------------------
# Project Configuration
# ----------------------------------
set(QAS_ROOT_PROJECT on)
set(QAS_INSTALL_NAME QasTool)
set(QAS_CURRENT_VERSION ${PROJECT_VERSION})
string(TOLOWER ${QAS_INSTALL_NAME} QAS_INSTALL_NAME_LOWER)
set(PROJECT_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake)

# ----------------------------------
# Build options
# ----------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

# ----------------------------------
# Add components
# ----------------------------------
include(${PROJECT_MODULES_DIR}/All.cmake)

add_subdirectory(src)

add_subdirectory(tools)

if(QAS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()


# ----------------------------------
# Install
# ----------------------------------
if(NOT QAS_SKIP_INSTALL)
    # Add install target
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${QAS_INSTALL_NAME_LOWER})
    set(_tools_dir ${CMAKE_INSTALL_PREFIX}/tools/${QAS_INSTALL_NAME_LOWER})

    # Add version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${QAS_INSTALL_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # Add configuration file
    configure_package_config_file(
        ${QAS_INSTALL_NAME}Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${QAS_INSTALL_NAME}Config.cmake"
        INSTALL_DESTINATION ${_install_dir}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    install(TARGETS core
        EXPORT QasToolTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(TARGETS qasc
        EXPORT QasToolTargets
        RUNTIME DESTINATION ${_tools_dir}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # Install cmake files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${QAS_INSTALL_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${QAS_INSTALL_NAME}ConfigVersion.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/QasToolMacros.cmake"
        DESTINATION ${_install_dir}
    )

    # Install cmake targets files
    install(EXPORT QasToolTargets
        NAMESPACE qastool::
        DESTINATION ${_install_dir}
    )

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${QAS_INSTALL_NAME_LOWER}
        FILES_MATCHING PATTERN "*.h"
    )
endif()
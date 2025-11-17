project(moc LANGUAGES CXX)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_compile_definitions(
    QT_MOC
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_FROM_BYTEARRAY
    QT_NO_COMPRESS
    QT_NO_FOREACH
)

if(MSVC)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    if(Qt6_VERSION VERSION_GREATER_EQUAL 6.10.0)
        find_package(Qt6 COMPONENTS CorePrivate REQUIRED)
        set(QT_CORE_PRIVATE_INCLUDES ${Qt6CorePrivate_INCLUDE_DIRS})
    else()
        set(QT_CORE_PRIVATE_INCLUDES ${Qt6Core_PRIVATE_INCLUDE_DIRS})
    endif()
elseif(QT_VERSION_MAJOR EQUAL 5)
    set(QT_CORE_PRIVATE_INCLUDES ${Qt5Core_PRIVATE_INCLUDE_DIRS})
endif()

include_directories(
    ${QT_CORE_PRIVATE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../tinycbor/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ----------------------------------
# moc.pri
# ----------------------------------
set(MOC_LIB_TARGET mocparser)

if(NOT DEFINED MOC_LIB_BUILD_SHARED OR NOT MOC_LIB_BUILD_SHARED)
    add_library(${MOC_LIB_TARGET} STATIC)
else()
    add_library(${MOC_LIB_TARGET} SHARED)
endif()

target_sources(${MOC_LIB_TARGET} PRIVATE
    preprocessor.h
    parser.h
    symbols.h
    token.h
    utils.h
    outputrevision.h
    cbordevice.h
    collectjson.h
    preprocessor.cpp
    parser.cpp
    token.cpp
    collectjson.cpp
)

target_link_libraries(${MOC_LIB_TARGET} PUBLIC Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(${MOC_LIB_TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# ----------------------------------
# moc.pro
# ----------------------------------
if((${QT_VERSION_MAJOR} EQUAL 5) AND QAS_BUILD_QT_MOC)
    set(MOC_EXE_TARGET ${PROJECT_NAME})

    add_executable(${MOC_EXE_TARGET} moc.h moc.cpp generator.h generator.cpp main.cpp)

    target_link_libraries(${MOC_EXE_TARGET} PRIVATE ${MOC_LIB_TARGET})

    set_target_properties(${MOC_EXE_TARGET}
        PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
    )
endif()
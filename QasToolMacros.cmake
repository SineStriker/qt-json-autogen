include(CMakeParseArguments)

if(TARGET qastool::qasc)
    get_target_property(QASTOOL_QASC_EXECUTABLE qastool::qasc LOCATION)
endif()

if(TARGET qastool::core)
    get_target_property(QASTOOL_INCLUDE_DIRS qastool::core INTERFACE_INCLUDE_DIRECTORIES)
endif()

# macro used to create the names of output files preserving relative dirs
macro(qas_make_output_file infile prefix ext outfile)
    string(LENGTH ${CMAKE_CURRENT_BINARY_DIR} _binlength)
    string(LENGTH ${infile} _infileLength)
    set(_checkinfile ${CMAKE_CURRENT_SOURCE_DIR})

    if(_infileLength GREATER _binlength)
        string(SUBSTRING "${infile}" 0 ${_binlength} _checkinfile)

        if(_checkinfile STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
            file(RELATIVE_PATH rel ${CMAKE_CURRENT_BINARY_DIR} ${infile})
        else()
            file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
        endif()
    else()
        file(RELATIVE_PATH rel ${CMAKE_CURRENT_SOURCE_DIR} ${infile})
    endif()

    if(CMAKE_HOST_WIN32 AND rel MATCHES "^([a-zA-Z]):(.*)$") # absolute path
        set(rel "${CMAKE_MATCH_1}_${CMAKE_MATCH_2}")
    endif()

    set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${rel}")
    string(REPLACE ".." "__" _outfile ${_outfile})
    get_filename_component(outpath ${_outfile} PATH)

    if(CMAKE_VERSION VERSION_LESS "3.14")
        get_filename_component(_outfile_ext ${_outfile} EXT)
        get_filename_component(_outfile_ext ${_outfile_ext} NAME_WE)
        get_filename_component(_outfile ${_outfile} NAME_WE)
        string(APPEND _outfile ${_outfile_ext})
    else()
        get_filename_component(_outfile ${_outfile} NAME_WLE)
    endif()

    file(MAKE_DIRECTORY ${outpath})
    set(${outfile} ${outpath}/${prefix}${_outfile}.${ext})
endmacro()

macro(qas_get_qasc_flags _qasc_flags)
    set(${_qasc_flags})
    get_directory_property(_inc_DIRS INCLUDE_DIRECTORIES)

    if(CMAKE_INCLUDE_CURRENT_DIR)
        list(APPEND _inc_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    foreach(_current ${_inc_DIRS})
        if("${_current}" MATCHES "\\.framework/?$")
            string(REGEX REPLACE "/[^/]+\\.framework" "" framework_path "${_current}")
            set(${_qasc_flags} ${${_qasc_flags}} "-F${framework_path}")
        else()
            set(${_qasc_flags} ${${_qasc_flags}} "-I${_current}")
        endif()
    endforeach()

    get_directory_property(_defines COMPILE_DEFINITIONS)

    foreach(_current ${_defines})
        set(${_qasc_flags} ${${_qasc_flags}} "-D${_current}")
    endforeach()

    if(WIN32)
        set(${_qasc_flags} ${${_qasc_flags}} -DWIN32)
    endif()

    if(MSVC)
        set(${_qasc_flags} ${${_qasc_flags}} --compiler-flavor=msvc)
    endif()
endmacro()

# helper macro to set up a moc rule
function(qas_create_qasc_command infile outfile qasc_flags qasc_options qasc_target qasc_depends)
    # Pass the parameters in a file.  Set the working directory to
    # be that containing the parameters file and reference it by
    # just the file name.  This is necessary because the moc tool on
    # MinGW builds does not seem to handle spaces in the path to the
    # file given with the @ syntax.
    get_filename_component(_qasc_outfile_name "${outfile}" NAME)
    get_filename_component(_qasc_outfile_dir "${outfile}" PATH)

    if(_qasc_outfile_dir)
        set(_qasc_working_dir WORKING_DIRECTORY ${_qasc_outfile_dir})
    endif()

    set(_qasc_parameters_file ${outfile}_parameters)
    set(_qasc_parameters ${qasc_flags} ${qasc_options} -o "${outfile}" "${infile}")
    string(REPLACE ";" "\n" _qasc_parameters "${_qasc_parameters}")

    if(qasc_target)
        set(_qasc_parameters_file ${_qasc_parameters_file}$<$<BOOL:$<CONFIGURATION>>:_$<CONFIGURATION>>)
        set(targetincludes "$<TARGET_PROPERTY:${qasc_target},INCLUDE_DIRECTORIES>")
        set(targetdefines "$<TARGET_PROPERTY:${qasc_target},COMPILE_DEFINITIONS>")

        set(targetincludes "$<$<BOOL:${targetincludes}>:-I$<JOIN:${targetincludes},\n-I>\n>")
        set(targetdefines "$<$<BOOL:${targetdefines}>:-D$<JOIN:${targetdefines},\n-D>\n>")

        file(GENERATE
            OUTPUT ${_qasc_parameters_file}
            CONTENT "${targetdefines}${targetincludes}${_qasc_parameters}\n"
        )

        set(targetincludes)
        set(targetdefines)
    else()
        file(WRITE ${_qasc_parameters_file} "${_qasc_parameters}\n")
    endif()

    set(_qasc_extra_parameters_file @${_qasc_parameters_file})

    if(WIN32)
        # Add Qt Core to PATH
        get_target_property(_loc Qt${QT_VERSION_MAJOR}::Core IMPORTED_LOCATION_RELEASE)
        get_filename_component(_dir ${_loc} DIRECTORY)
        set(_cmd
            COMMAND set "Path=${_dir}\;%Path%\;"
            COMMAND ${QASTOOL_QASC_EXECUTABLE} ${_qasc_extra_parameters_file}
        )
    else()
        set(_cmd
            COMMAND ${QASTOOL_QASC_EXECUTABLE} ${_qasc_extra_parameters_file}
        )
    endif()

    add_custom_command(OUTPUT ${outfile}
        ${_cmd}
        DEPENDS ${infile} ${qasc_depends}
        ${_qasc_working_dir}
        VERBATIM
    )
endfunction()

function(qas_generate_moc infile outfile)
    # get include dirs and flags
    qas_get_qasc_flags(qasc_flags)
    get_filename_component(abs_infile ${infile} ABSOLUTE)
    set(_outfile "${outfile}")

    if(NOT IS_ABSOLUTE "${outfile}")
        set(_outfile "${CMAKE_CURRENT_BINARY_DIR}/${outfile}")
    endif()

    if("x${ARGV2}" STREQUAL "xTARGET")
        set(qasc_target ${ARGV3})
    endif()

    qas_create_qasc_command(${abs_infile} ${_outfile} "${qasc_flags}" "" "${qasc_target}" "")
endfunction()

function(qas_wrap_cpp outfiles)
    # get include dirs
    qas_get_qasc_flags(qasc_flags)

    set(options)
    set(oneValueArgs TARGET)
    set(multiValueArgs OPTIONS DEPENDS)

    cmake_parse_arguments(_WRAP_CPP "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(qasc_files ${_WRAP_CPP_UNPARSED_ARGUMENTS})
    set(qasc_options ${_WRAP_CPP_OPTIONS})
    set(qasc_target ${_WRAP_CPP_TARGET})
    set(qasc_depends ${_WRAP_CPP_DEPENDS})

    foreach(it ${qasc_files})
        get_filename_component(it ${it} ABSOLUTE)
        qas_make_output_file(${it} qasc_ cpp outfile)
        qas_create_qasc_command(${it} ${outfile} "${qasc_flags}" "${qasc_options}" "${qasc_target}" "${qasc_depends}")
        list(APPEND ${outfiles} ${outfile})
    endforeach()

    set(${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()
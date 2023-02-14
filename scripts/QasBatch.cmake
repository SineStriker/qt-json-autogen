# Usage:
# cmake
# -D INCLUDE_DIRECTORIES=
# -D COMPILE_DEFINITIONS=
# -D SOURCES=
# -D QASC_COMMAND=
# -D QASC_CPP=
# -P QasBatch.cmake

set(_macros)
set(_incdirs)
set(_headers)

foreach(_macro ${COMPILE_DEFINITIONS})
    list(APPEND _macros "-D${_macro}")
endforeach()

foreach(_dir ${INCLUDE_DIRECTORIES})
    list(APPEND _incdirs "-I${_dir}")
endforeach()

foreach(_file ${SOURCES})
    get_filename_component(_suffix ${_file} EXT)
    string(TOLOWER ${_suffix} _lower_suffix)

    if((${_lower_suffix} STREQUAL .h) OR(${_lower_suffix} STREQUAL .hpp))
        list(APPEND _headers ${_file})
    endif()
endforeach()

get_filename_component(_bin_dir ${QASC_CPP} DIRECTORY)
set(_content)

foreach(_file ${_headers})
    get_filename_component(_name ${_file} NAME_WE)
    string(SHA256 _hash ${_file})
    set(_output "${_bin_dir}/qasc_${_name}_${_hash}.cpp")

    if(EXISTS ${_output})
        file(REMOVE ${_output})
    endif()

    execute_process(
        COMMAND "${QASC_COMMAND}" ${_file} ${_macros} ${_incdirs} -o ${_output} --debug
        WORKING_DIRECTORY ${_bin_dir}
    )

    file(SIZE ${_output} _size)

    if(${_size} STREQUAL 0)
        file(REMOVE ${_output})
    else()
        set(_content "${_content}#include \"${_output}\"\n")
    endif()
endforeach()

if(EXISTS ${QASC_CPP})
    file(REMOVE ${QASC_CPP})
endif()

file(WRITE ${QASC_CPP} ${_content})
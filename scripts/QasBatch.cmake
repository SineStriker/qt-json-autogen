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
    string(RANDOM LENGTH 10 _rand)
    set(_output "qasc_${_name}_${_rand}.cpp")
    set(_content "${_content}#include \"${_output}\"\n")

    execute_process(
        COMMAND "${QASC_COMMAND}" ${_file} ${_macros} ${_incdirs} -o ${_bin_dir}/${_output}
        WORKING_DIRECTORY ${_bin_dir}
    )
endforeach()

if(EXISTS ${QASC_CPP})
    file(REMOVE ${QASC_CPP})
endif()

file(WRITE ${QASC_CPP} ${_content})
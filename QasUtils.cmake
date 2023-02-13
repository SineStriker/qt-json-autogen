include_guard(DIRECTORY)

function(qas_auto_gen _target)
    set(_tmp_dir ${CMAKE_CURRENT_BINARY_DIR}/${_target}_autogen_qas)
    set(_lst_file ${_tmp_dir}/${_target}_src_lst_file)
    set(_qas_cpp ${_tmp_dir}/qas_compilations.cpp)
    make_directory(${_tmp_dir})

    # Flush all source files to filesystem
    file(GENERATE
        OUTPUT ${_lst_file}
        CONTENT $<TARGET_PROPERTY:${_target},SOURCES>
    )

    # Add temp source to target
    file(TOUCH ${_qas_cpp})
    target_sources(${_target} PRIVATE ${_qas_cpp})

    # Pre-process the source files
    add_custom_command(
        TARGET ${_target}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E cat ${_lst_file}
    )
endfunction()

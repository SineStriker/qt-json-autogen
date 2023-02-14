include_guard(DIRECTORY)

set(QASC_COMMAND "${CMAKE_CURRENT_LIST_DIR}/scripts/qasc" CACHE STRING "Qasc command" FORCE)
set(QAS_BATCH_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/scripts/QasBatch.cmake" CACHE STRING "Qas batch script" FORCE)

function(qas_auto_gen _target)
    set(_tmp_dir ${CMAKE_CURRENT_BINARY_DIR}/${_target}_autogen_qas)
    set(_qas_cpp ${_tmp_dir}/qasc_compilations.cpp)
    make_directory(${_tmp_dir})

    # Add temp source to target
    file(TOUCH ${_qas_cpp})
    target_sources(${_target} PRIVATE ${_qas_cpp})

    # Add autogen target
    set(_qasc_target ${_target}_autogen_qas)
    add_custom_target(
        ${_qasc_target}
        COMMAND ${CMAKE_COMMAND}
        "-DINCLUDE_DIRECTORIES=$<TARGET_PROPERTY:${_target},INCLUDE_DIRECTORIES>"
        "-DCOMPILE_DEFINITIONS=$<TARGET_PROPERTY:${_target},COMPILE_DEFINITIONS>"
        "-DSOURCES=$<TARGET_PROPERTY:${_target},SOURCES>"
        "-DQASC_COMMAND=${QASC_COMMAND}"
        "-DQASC_CPP=${_qas_cpp}"
        -P "${QAS_BATCH_SCRIPT}"
        DEPENDS "$<TARGET_PROPERTY:${_target},SOURCES>"
    )

    # Set dependencies
    add_dependencies(${_target} ${_qasc_target})
endfunction()

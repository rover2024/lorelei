# Guest architecture: x86_64
set(LORE_GUEST_ARCH "x86_64")
set(LORE_GUEST_FIXED_REGISTER "r11")

if(NOT DEFINED LORE_HOST_ARCH)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        set(LORE_HOST_ARCH "x86_64")
        set(LORE_HOST_FIXED_REGISTER "r11")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(LORE_HOST_ARCH "aarch64")
        set(LORE_HOST_FIXED_REGISTER "x16")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
        set(LORE_HOST_ARCH "riscv64")
        set(LORE_HOST_FIXED_REGISTER "t1")
    endif()
endif()

if(NOT DEFINED LORE_THUNK_SOURCES_DIR)
    set(LORE_THUNK_GENERATE on)
    set(LORE_THUNK_SOURCES_DIR ${LORE_BUILD_BASE_DIR}/src/thunks)
else()
    set(LORE_THUNK_GENERATE off)
endif()

#[[
    Link clang libraries to a target.

    lore_link_clang(<target> <scope>)
#]]
function(lore_link_clang _target _scope)
    find_package(Clang REQUIRED)
    target_include_directories(${_target} SYSTEM ${_scope} ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_link_directories(${_target} ${_scope} ${LLVM_LIBRARY_DIRS})
    target_compile_definitions(${_target} ${_scope} ${LLVM_DEFINITIONS})
    target_link_libraries(${_target} ${_scope}
        clangAST
        clangBasic
        clangFrontend
        clangSerialization
        clangTooling
        clangASTMatchers
    )
endfunction()

#[[
    Add a pkgconfig dependency to a target.

    lore_add_pkgconfig_dependency(<target> <pkg>)
#]]
function(lore_add_pkgconfig_dependency _target _pkg)
    set(options REQUIRED QUIET)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_REQUIRED)
        set(_opt REQUIRED)
    elseif(FUNC_QUIET)
        set(_opt QUIET)
    else()
        set(_opt)
    endif()

    string(TOUPPER ${_pkg} _pkg_upper)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(${_pkg_upper} ${_opt} ${_pkg})

    if(${_pkg_upper}_FOUND)
        target_include_directories(${PROJECT_NAME} PRIVATE ${${_pkg_upper}_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${${_pkg_upper}_LIBRARIES})
    else()
        message(FATAL_ERROR "Failed to find ${_pkg}")
    endif()
endfunction()

#[[
    Generate a global header file for a target.

    lore_generate_global_header(<target> <rel_path>)
#]]
function(lore_generate_global_header _target _rel_path)
    set(options)
    set(oneValueArgs PREFIX)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT FUNC_PREFIX)
        string(TOUPPER ${_target} _prefix)
        string(MAKE_C_IDENTIFIER ${_prefix} _prefix)
    else()
        set(_prefix ${FUNC_PREFIX})
    endif()

    set(_content "#pragma once

#ifdef ${_prefix}_LIBRARY
#  define ${_prefix}_EXPORT __attribute__((visibility(\"default\")))
#else
#  define ${_prefix}_EXPORT
#endif
")

    set(_out_dir ${LORE_BUILD_INCLUDE_DIR}/${_rel_path})
    set(_out_file "${_out_dir}/Global.h")
    file(MAKE_DIRECTORY ${_out_dir})
    file(WRITE ${_out_file} ${_content})
    target_include_directories(${_target} PUBLIC $<BUILD_INTERFACE:${_out_dir}>)

    if(LORE_INSTALL)
        install(FILES ${_out_file}
            DESTINATION ${LORE_INSTALL_INCLUDE_DIR}/${_rel_path}
        )
    endif()
endfunction()

function(lore_guest_thunk_disable_register _target)
    target_compile_options(${_target} PRIVATE "-ffixed-${LORE_GUEST_FIXED_REGISTER}")
endfunction()

function(lore_host_thunk_disable_register _target)
    target_compile_options(${_target} PRIVATE "-ffixed-${LORE_HOST_FIXED_REGISTER}")
endfunction()

function(lore_generate_thunk _name _input_file _out_file _config_file)
    set(options)
    set(oneValueArgs OUT_CALLBACKS_FILE)
    set(multiValueArgs PRE_INCLUDE_FILES PLUGINS EXTRA_ARGS DEPENDS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_tool $<TARGET_FILE:LoreTLC>)
    set(_args)

    list(APPEND _args -o ${_out_file} -s ${_config_file})

    if(FUNC_OUT_CALLBACKS_FILE)
        list(APPEND _args --out-callbacks=${FUNC_OUT_CALLBACKS_FILE})
    endif()

    foreach(_file IN LISTS FUNC_PRE_INCLUDE_FILES)
        list(APPEND _args --preinc=${_file})
    endforeach()

    foreach(_plugin IN LISTS FUNC_PLUGINS)
        list(APPEND _args --plugin=${_plugin})
    endforeach()

    list(APPEND _args ${_input_file})

    list(APPEND _args "--" "-xc++" "-I${LORE_SOURCE_DIR}/include" "-I${QMSETUP_BUILD_DIR}/include" -I/usr/lib/llvm-18/lib/clang/18/include)

    if(FUNC_EXTRA_ARGS)
        list(APPEND _args ${FUNC_EXTRA_ARGS})
    endif()

    get_filename_component(_dir ${_out_file} DIRECTORY)
    file(MAKE_DIRECTORY ${_dir})

    add_custom_command(OUTPUT ${_out_file}
        COMMAND ${_tool} ${_args}
        DEPENDS ${_tool} ${_input_file} ${_config_file} ${FUNC_DEPENDS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_custom_target(thunk_gen_${_name} DEPENDS ${_out_file})

    if(TARGET thunk_gen_all)
        add_dependencies(thunk_gen_all thunk_gen_${_name})
    endif()
endfunction()

function(lore_add_guest_thunk _target)
    set(_dir ${LORE_GUEST_ARCH}-linux-gnu)
    lore_add_library(${_target} SHARED
        LIBRARY_DIRECTORY ${_dir}
        ${ARGN}
    )
    _lore_configure_thunk(${_target} ${_dir})

    if(_target MATCHES "(.+)_GTL")
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${CMAKE_MATCH_1})
    endif()
endfunction()

function(lore_add_host_thunk _target)
    if(LORE_HOST_ARCH STREQUAL "x86_64")
        set(_dir amd64-linux-gnu)
    else()
        set(_dir ${LORE_HOST_ARCH}-linux-gnu)
    endif()

    lore_add_library(${_target} SHARED
        LIBRARY_DIRECTORY ${_dir}
        ${ARGN}
    )
    _lore_configure_thunk(${_target} ${_dir})
endfunction()

function(_lore_configure_thunk _target _dir)
    get_target_property(_links ${_target} POST_LINK_ALIAS)
    set_target_properties(${_target} PROPERTIES
        INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../../lib"
    )

    if(_links)
        foreach(_link IN LISTS _links)
            add_custom_command(TARGET ${_target} POST_BUILD
                COMMAND ln -sf $<TARGET_FILE_NAME:${_target}> ${_link}
                WORKING_DIRECTORY $<TARGET_FILE_DIR:${_target}>
            )
        endforeach()
    endif()

    if(LORE_INSTALL AND NOT LORE_SKIP_INSTALL)
        if(_links)
            foreach(_link IN LISTS _links)
                install(CODE "
                    execute_process(
                        COMMAND ln -sf $<TARGET_FILE_NAME:${_target}> ${_link}
                        WORKING_DIRECTORY \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${_dir}
                    )
                ")
            endforeach()
        endif()
    endif()
endfunction()

macro(lore_add_resource _target _in_file)
    qm_import(private/Generate)
    set(_res_out_file)
    qm_make_output_file(${_in_file} "res_" "c" _res_out_file)
    qm_add_binary_resource(${_in_file} ${_res_out_file} USE_SCRIPT)
    target_sources(${_target} PRIVATE ${_res_out_file})
endmacro()

function(lore_add_auto_test _src)
    get_filename_component(_name ${_src} NAME_WE)
    add_executable(${_name} ${_src})
    target_link_libraries(${_name} PRIVATE Boost::unit_test_framework)
    add_test(NAME ${_name} COMMAND $<TARGET_FILE:${_name}>)
endfunction()
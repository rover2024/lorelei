# Guest architecture: x86_64
set(LORE_GUEST_ARCH "x86_64")
set(LORE_GUEST_FIXED_REGISTER "r11")

if(NOT LORE_HOST_ARCH)
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

    if(NOT EXISTS ${_out_file})
        file(MAKE_DIRECTORY ${_out_dir})
        file(WRITE ${_out_file} ${_content})
    endif()

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

function(lore_add_guest_thunk _target)
    set(_dir ${LORE_GUEST_ARCH}-LoreGTL)
    lore_add_library(${_target} SHARED
        LIBRARY_DIRECTORY ${_dir}
        ${ARGN}
    )
    _lore_configure_thunk(${_target} ${_dir} ${ARGN})

    if(_target MATCHES "(.+)_GTL")
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${CMAKE_MATCH_1})
    endif()
endfunction()

function(lore_add_host_thunk _target)
    set(_dir ${LORE_HOST_ARCH}-LoreHTL)
    lore_add_library(${_target} SHARED
        LIBRARY_DIRECTORY ${_dir}
        ${ARGN}
    )
    _lore_configure_thunk(${_target} ${_dir} ${ARGN})
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

# ----------------------------------
# Internal Functions
# ----------------------------------
function(_lore_configure_thunk _target _dir)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs POST_LINK_ALIAS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(FUNC_POST_LINK_ALIAS)
        foreach(_link IN LISTS FUNC_POST_LINK_ALIAS)
            add_custom_command(TARGET ${_target} POST_BUILD
                COMMAND ln -sf $<TARGET_FILE_NAME:${_target}> ${_link}
                WORKING_DIRECTORY $<TARGET_FILE_DIR:${_target}>
            )
        endforeach()
    endif()

    if(LORE_INSTALL AND NOT LORE_SKIP_INSTALL)
        if(FUNC_POST_LINK_ALIAS)
            foreach(_link IN LISTS FUNC_POST_LINK_ALIAS)
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
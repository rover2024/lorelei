# Guest architecture: x86_64
set(LORE_GUEST_ARCH "x86_64")

# Host architecture from CMake's own target processor rather than the compiler's -dumpmachine: gcc
# bakes the target into the driver, but clang does not (its -dumpmachine ignores -target), so parsing
# the compiler output would misdetect a clang cross build. CMAKE_SYSTEM_PROCESSOR is set natively and
# by the cross toolchain files, so this is correct for both compilers and for native and cross builds.
if(NOT LORE_HOST_ARCH)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        set(LORE_HOST_ARCH "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64")
        set(LORE_HOST_ARCH "aarch64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
        set(LORE_HOST_ARCH "riscv64")
    endif()
endif()

#[[
    Link clang libraries to a target.

    lore_link_clang(<target> <scope>)
#]]
function(lore_link_clang _target _scope)
    find_package(Clang REQUIRED)
    target_include_directories(${_target} SYSTEM ${_scope} ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_compile_definitions(${_target} ${_scope} ${LLVM_DEFINITIONS})

    if(LORE_STATIC_LLVM)
        # Self-contained toolchain: link the static Clang archives against static LLVM components, so
        # the tool has no runtime dependency on libLLVM.so / libclang-cpp.so. The Clang CMake targets
        # pull in the shared libLLVM, so bypass them and link the .a directly, resolved in one group.
        # LLVM_LIBRARY_DIRS and LLVM_TOOLS_BINARY_DIR come from find_package(Clang) above, so nothing
        # here hard-codes the LLVM version.
        execute_process(COMMAND ${LLVM_TOOLS_BINARY_DIR}/llvm-config --link-static --system-libs
            OUTPUT_VARIABLE _llvm_syslibs OUTPUT_STRIP_TRAILING_WHITESPACE)
        separate_arguments(_llvm_syslibs UNIX_COMMAND "${_llvm_syslibs}")

        # Glob the archives that actually exist rather than trusting `llvm-config --libs`, which lists
        # optional components (e.g. Polly) that the distribution does not ship a static library for.
        file(GLOB _clang_archives "${LLVM_LIBRARY_DIRS}/libclang*.a")
        file(GLOB _llvm_archives "${LLVM_LIBRARY_DIRS}/libLLVM*.a")

        target_link_directories(${_target} ${_scope} ${LLVM_LIBRARY_DIRS})
        # --start-group: the Clang and LLVM archives reference each other both ways, so one pass is
        # not enough. The linker only extracts the objects actually needed, so unused components
        # (e.g. most target backends) do not bloat the binary.
        target_link_libraries(${_target} ${_scope}
            -Wl,--start-group ${_clang_archives} ${_llvm_archives} -Wl,--end-group ${_llvm_syslibs})
    else()
        target_link_directories(${_target} ${_scope} ${LLVM_LIBRARY_DIRS})
        target_link_libraries(${_target} ${_scope}
            clangAST
            clangBasic
            clangFrontend
            clangSerialization
            clangTooling
            clangASTMatchers
        )
    endif()
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

macro(lore_add_resource _target _in_file)
    qm_import(private/Generate)
    set(_res_out_file)
    qm_make_output_file(${_in_file} "res_" "c" _res_out_file)
    qm_add_binary_resource(${_in_file} ${_res_out_file} USE_SCRIPT)
    target_sources(${_target} PRIVATE ${_res_out_file})
endmacro()
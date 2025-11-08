qm_import(Preprocess)

add_custom_target(thunk_gen_all)

function(lore_include_recursive _target _scope)
    foreach(_dir ${ARGN})
        if(IS_DIRECTORY ${_dir})
            target_include_directories(${_target} ${_scope} ${_dir})

            file(GLOB_RECURSE _subdirs LIST_DIRECTORIES TRUE RELATIVE ${_dir} ${_dir}/*)

            foreach(_subdir IN LISTS _subdirs)
                if(IS_DIRECTORY ${_dir}/${_subdir})
                    target_include_directories(${_target} ${_scope} ${_dir}/${_subdir})
                endif()
            endforeach()
        endif()
    endforeach()
endfunction()

function(lore_configure_target_common _target)
    target_include_directories(${_target} PUBLIC $<BUILD_INTERFACE:${LORELEI_SOURCE_DIR}/include>)
    target_include_directories(${_target} PUBLIC $<BUILD_INTERFACE:${QMSETUP_BUILD_DIR}/include>)
    lore_include_recursive(${_target} PRIVATE .)
    target_compile_features(${_target} PUBLIC cxx_std_17)

    if(LORELEI_INSTALL)
        target_include_directories(${_target} PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
    endif()

    qm_export_defines(${_target})
endfunction()

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

    set(_out_dir ${QMSETUP_BUILD_DIR}/include/${_rel_path})
    set(_out_file "${_out_dir}/Global.h")
    file(MAKE_DIRECTORY ${_out_dir})
    file(WRITE ${_out_file} ${_content})
    target_include_directories(${_target} PRIVATE ${_out_dir})

    if(LORELEI_INSTALL)
        install(FILES ${_out_file}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${_rel_path}
        )
    endif()
endfunction()

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
    endif()
endfunction()

function(lore_configure_library _target)
    if(LORELEI_INSTALL)
        target_include_directories(${_target} PUBLIC "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
        install(TARGETS ${_target}
            EXPORT ${LORELEI_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )
    endif()
endfunction()

function(lore_configure_tool _target)
    if(LORELEI_INSTALL)
        if(LORELEI_VCPKG_TOOLS_HINT)
            set(_tools_dir tools/${LORELEI_INSTALL_NAME})
        else()
            set(_tools_dir ${CMAKE_INSTALL_BINDIR})
        endif()

        install(TARGETS ${_target}
            EXPORT ${LORELEI_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        )
    endif()
endfunction()

function(lore_configure_plugin _target _category)
    set(options INSTALL)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)

    set_target_properties(${_target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins/${_category}
        INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../../../lib"
    )

    if(FUNC_INSTALL AND LORELEI_INSTALL)
        install(TARGETS ${_target}
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/plugins/${_category}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/plugins/${_category}" OPTIONAL
        )
    endif()
endfunction()

function(lore_guest_thunk_disable_register _target)
    target_compile_options(${_target} PRIVATE "-ffixed-${LORELEI_GUEST_FIXED_REGISTER}")
endfunction()

function(lore_host_thunk_disable_register _target)
    target_compile_options(${_target} PRIVATE "-ffixed-${LORELEI_HOST_FIXED_REGISTER}")
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

    list(APPEND _args "--" "-xc++" "-I${LORELEI_SOURCE_DIR}/include" -I/usr/lib/llvm-18/lib/clang/18/include)

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
    add_dependencies(thunk_gen_all thunk_gen_${_name})
endfunction()

function(lore_configure_guest_thunk _target)
    if(_target MATCHES "(.+)_GTL")
        set_target_properties(${_target} PROPERTIES OUTPUT_NAME ${CMAKE_MATCH_1})
    endif()

    _lore_configure_thunk(${_target} ${LORELEI_GUEST_ARCH}-loreg-linux-gnu)
endfunction()

function(lore_configure_host_thunk _target)
    _lore_configure_thunk(${_target} ${LORELEI_HOST_ARCH}-loreh-linux-gnu)
endfunction()

function(_lore_configure_thunk _target _dir)
    get_target_property(_links ${_target} POST_LINK_ALIAS)
    set_target_properties(${_target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_dir}
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

    if(LORELEI_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${LORELEI_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${_dir}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/${_dir}" OPTIONAL
        )

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
set(GTL ${PROJECT_NAME}_GTL)
set(HTL ${PROJECT_NAME}_HTL)

get_filename_component(_dir_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(_thunk_src_dir ${LORE_THUNK_SOURCE_DIR}/${_dir_name})
set(_thunk_data_dir ${LORE_THUNK_DATA_DIR}/${_dir_name})

set(_desc_file Desc.h)
set(_manifest_guest_file Manifest_guest.cpp)
set(_manifest_host_file Manifest_host.cpp)
set(_symbols_config Symbols.conf)
set(_stat_file ${_thunk_data_dir}/Stat.json)

# Fixed variables
set(GTL_src ${_thunk_src_dir}/Thunk_guest.cpp)
set(HTL_src ${_thunk_src_dir}/Thunk_host.cpp)

# Optional pre-defined variables:
set(ALL_extra_args)
set(STAT_extra_args)
set(GTL_extra_args)
set(HTL_extra_args)
set(ALL_extra_includes)
set(STAT_extra_includes)
set(GTL_extra_includes)
set(HTL_extra_includes)
set(GTL_extra_links)
set(HTL_extra_links)
set(GTL_extra_options)
set(HTL_extra_options)
set(GTL_force_links)
set(HTL_force_links)
set(GTL_alias)
set(HTL_alias)
set(PLUGIN_target)

macro(add_thunk)
    # Add TLC stat target
    if(TRUE)
        set(_extra_args)
        set(_options)

        # Add extra arguments
        if(ALL_extra_args)
            list(APPEND _extra_args ${ALL_extra_args})
        endif()

        if(ALL_extra_includes)
            foreach(_item IN LISTS ALL_extra_includes)
                list(APPEND _extra_args -I${_item})
            endforeach()
        endif()

        if(STAT_extra_args)
            list(APPEND _extra_args ${STAT_extra_args})
        endif()

        if(STAT_extra_includes)
            foreach(_item IN LISTS STAT_extra_includes)
                list(APPEND _extra_args -I${_item})
            endforeach()
        endif()

        if(DEFINED LORE_TLC_EXTRA_ARGS)
            list(APPEND _extra_args ${LORE_TLC_EXTRA_ARGS})
        endif()

        # Add target
        _tlc_stat(${PROJECT_NAME} ${_desc_file} ${_stat_file} ${_symbols_config}
            EXTRA_ARGS ${_extra_args}
            DEPENDS ${_desc_file} ${_symbols_config}
            ${_options}
        )
    endif()

    # Add TLC generate target (Guest)
    if(LORE_BUILD_GUEST_TARGETS)
        if(TRUE)
            set(_extra_args)
            set(_options)

            # Add extra arguments
            if(ALL_extra_args)
                list(APPEND _extra_args ${ALL_extra_args})
            endif()

            if(ALL_extra_includes)
                foreach(_item IN LISTS ALL_extra_includes)
                    list(APPEND _extra_args -I${_item})
                endforeach()
            endif()

            if(GTL_extra_args)
                list(APPEND _extra_args ${GTL_extra_args})
            endif()

            if(GTL_extra_includes)
                foreach(_item IN LISTS GTL_extra_includes)
                    list(APPEND _extra_args -I${_item})
                endforeach()
            endif()

            if(DEFINED LORE_TLC_EXTRA_ARGS)
                list(APPEND _extra_args ${LORE_TLC_EXTRA_ARGS})
            endif()

            # Add plugin
            if(PLUGIN_target)
                list(APPEND _options PLUGINS $<TARGET_FILE:${PLUGIN_target}>)
            endif()

            # Add target
            _tlc_generate(${GTL} ${_manifest_guest_file} ${GTL_src} ${_stat_file} GUEST
                EXTRA_ARGS ${_extra_args}
                DEPENDS ${_manifest_guest_file} ${_stat_file}
                ${_options}
            )
        endif()

        lore_add_guest_thunk(${GTL})
        target_sources(${GTL} PRIVATE ${GTL_src})

        if(GTL_alias)
            set_target_properties(${GTL} PROPERTIES POST_LINK_ALIAS "${GTL_alias}")
        endif()

        lore_guest_thunk_disable_register(${GTL})
        target_link_libraries(${GTL} PUBLIC LoreGuestRT)
        target_include_directories(${GTL} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

        if(ALL_extra_includes)
            target_include_directories(${GTL} PRIVATE ${ALL_extra_includes})
        endif()

        if(GTL_extra_includes)
            target_include_directories(${GTL} PRIVATE ${GTL_extra_includes})
        endif()

        if(GTL_extra_links)
            target_link_libraries(${GTL} PRIVATE ${GTL_extra_links})
        endif()

        if(GTL_extra_options)
            target_compile_options(${GTL} PRIVATE ${GTL_extra_options})
        endif()

        if(GTL_force_links)
            target_link_options(${GTL} PRIVATE -Wl,--no-as-needed)
            target_link_libraries(${GTL} PRIVATE ${GTL_force_links})
        endif()
    endif()

    # Add TLC generate target (Host)
    if(TRUE)
        if(TRUE)
            set(_extra_args)
            set(_options)

            # Add extra arguments
            if(ALL_extra_args)
                list(APPEND _extra_args ${ALL_extra_args})
            endif()

            if(ALL_extra_includes)
                foreach(_item IN LISTS ALL_extra_includes)
                    list(APPEND _extra_args -I${_item})
                endforeach()
            endif()

            if(HTL_extra_args)
                list(APPEND _extra_args ${HTL_extra_args})
            endif()

            if(HTL_extra_includes)
                foreach(_item IN LISTS HTL_extra_includes)
                    list(APPEND _extra_args -I${_item})
                endforeach()
            endif()

            if(DEFINED LORE_TLC_EXTRA_ARGS)
                list(APPEND _extra_args ${LORE_TLC_EXTRA_ARGS})
            endif()

            # Add plugin
            if(PLUGIN_target)
                list(APPEND _options PLUGINS $<TARGET_FILE:${PLUGIN_target}>)
            endif()

            # Add target
            _tlc_generate(${HTL} ${_manifest_host_file} ${HTL_src} ${_stat_file} HOST
                EXTRA_ARGS ${_extra_args}
                DEPENDS ${_manifest_host_file} ${_stat_file}
                ${_options}
            )
        endif()

        lore_add_host_thunk(${HTL})
        target_sources(${HTL} PRIVATE ${HTL_src})

        if(HTL_alias)
            set_target_properties(${HTL} PROPERTIES POST_LINK_ALIAS "${HTL_alias}")
        endif()

        lore_host_thunk_disable_register(${HTL})
        target_link_libraries(${HTL} PUBLIC LoreHostRT)
        target_include_directories(${HTL} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

        if(ALL_extra_includes)
            target_include_directories(${HTL} PRIVATE ${ALL_extra_includes})
        endif()

        if(HTL_extra_includes)
            target_include_directories(${HTL} PRIVATE ${HTL_extra_includes})
        endif()

        if(HTL_extra_links)
            target_link_libraries(${HTL} PRIVATE ${HTL_extra_links})
        endif()

        if(HTL_extra_options)
            target_compile_options(${HTL} PRIVATE ${HTL_extra_options})
        endif()

        if(HTL_force_links)
            target_link_options(${HTL} PRIVATE -Wl,--no-as-needed)
            target_link_libraries(${HTL} PRIVATE ${HTL_force_links})
        endif()
    endif()
endmacro()

# ----------------------------------
# Internal Functions
# ----------------------------------
function(_tlc_stat _name _input_file _out_file _config_file)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs EXTRA_ARGS DEPENDS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_tool $<TARGET_FILE:LoreTLC>)
    set(_args)
    list(APPEND _args -o ${_out_file} -c ${_config_file})

    list(APPEND _args ${_input_file})
    list(APPEND _args "--"
        -xc++ "-I${LORE_SOURCE_DIR}/include" "-I${LORE_BUILD_INCLUDE_DIR}"
    )

    if(FUNC_EXTRA_ARGS)
        list(APPEND _args ${FUNC_EXTRA_ARGS})
    endif()

    # Make output directory
    get_filename_component(_dir ${_out_file} DIRECTORY)
    file(MAKE_DIRECTORY ${_dir})

    add_custom_command(OUTPUT ${_out_file}
        COMMAND ${_tool} "stat" ${_args}
        DEPENDS ${_tool} ${_input_file} ${_config_file} ${FUNC_DEPENDS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_custom_target(TLC_stat_${_name} DEPENDS ${_out_file})

    if(TARGET TLC_stat_all)
        add_dependencies(TLC_stat_all TLC_stat_${_name})
    endif()
endfunction()

function(_tlc_generate _name _input_file _out_file _input_stat_file)
    set(options HOST GUEST)
    set(oneValueArgs)
    set(multiValueArgs PLUGINS EXTRA_ARGS DEPENDS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_tool $<TARGET_FILE:LoreTLC>)
    set(_args)
    list(APPEND _args -o ${_out_file} -s ${_input_stat_file})

    foreach(_plugin IN LISTS FUNC_PLUGINS)
        list(APPEND _args -Xclang -load -Xclang ${_plugin})
    endforeach()

    if(FUNC_HOST)
        list(APPEND _args "-m" "host")
    else()
        list(APPEND _args "-m" "guest")
    endif()

    list(APPEND _args ${_input_file})
    list(APPEND _args "--" "-xc++"
        "-I${LORE_SOURCE_DIR}/include" "-I${LORE_BUILD_INCLUDE_DIR}"
    )

    # Add current source directory to include paths
    list(APPEND _args "-I${CMAKE_CURRENT_SOURCE_DIR}")

    if(FUNC_EXTRA_ARGS)
        list(APPEND _args ${FUNC_EXTRA_ARGS})
    endif()

    # Make output directory
    get_filename_component(_dir ${_out_file} DIRECTORY)
    file(MAKE_DIRECTORY ${_dir})

    add_custom_command(OUTPUT ${_out_file}
        COMMAND ${_tool} "generate" ${_args}
        DEPENDS ${_tool} ${_input_file} ${_input_stat_file} ${FUNC_DEPENDS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_custom_target(TLC_generate_${_name} DEPENDS ${_out_file})

    if(TARGET TLC_generate_all)
        add_dependencies(TLC_generate_all TLC_generate_${_name})
    endif()
endfunction()

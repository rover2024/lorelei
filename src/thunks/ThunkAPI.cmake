set(GTL ${PROJECT_NAME}_GTL)
set(HTL ${PROJECT_NAME}_HTL)

set(GTL_src ${LORELEI_THUNK_SOURCES_DIR}/${PROJECT_NAME}/Thunk_guest.cpp)
set(HTL_src ${LORELEI_THUNK_SOURCES_DIR}/${PROJECT_NAME}/Thunk_host.cpp)

# Optional pre-defined variables:
set(ALL_extra_args)
set(GTL_extra_args)
set(HTL_extra_args)
set(ALL_extra_includes)
set(GTL_extra_includes)
set(HTL_extra_includes)
set(GTL_extra_links)
set(HTL_extra_links)
set(GTL_force_links)
set(HTL_force_links)
set(GTL_alias)
set(HTL_alias)
set(PLUGIN_target)

macro(add_thunk)
    if(LORELEI_ENABLE_GUEST)
        if(LORELEI_THUNK_GENERATE)
            set(_extra_args)
            set(_options)

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

            if(PLUGIN_target)
                list(APPEND _options PLUGINS $<TARGET_FILE:${PLUGIN_target}>)
            endif()

            lore_generate_thunk(${GTL} Thunk_manifest_guest.cpp ${GTL_src} Thunk_config.conf
                EXTRA_ARGS ${_extra_args} ${LORELEI_GUEST_TLC_ARGS}
                PRE_INCLUDE_FILES Thunk_procs_desc.h
                DEPENDS Thunk_procs_desc.h Thunk_config_symbols.conf
                ${_options}
            )
        endif()

        add_library(${GTL} SHARED ${GTL_src})

        if(GTL_alias)
            set_target_properties(${GTL} PROPERTIES POST_LINK_ALIAS "${GTL_alias}")
        endif()

        lore_configure_target_common(${GTL})
        lore_configure_guest_thunk(${GTL})
        lore_guest_thunk_disable_register(${GTL})
        target_link_libraries(${GTL} PUBLIC LoreGuestRT)

        if(GTL_extra_includes)
            target_include_directories(${GTL} PRIVATE ${GTL_extra_includes})
        endif()

        if(GTL_extra_links)
            target_link_libraries(${GTL} PRIVATE ${GTL_extra_links})
        endif()

        if(GTL_force_links)
            target_link_options(${GTL} PRIVATE -Wl,--no-as-needed)
            target_link_libraries(${GTL} PRIVATE ${GTL_force_links})
        endif()
    endif()

    if(LORELEI_ENABLE_HOST)
        if(LORELEI_THUNK_GENERATE)
            set(_extra_args)
            set(_options)

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

            if(PLUGIN_target)
                list(APPEND _options PLUGINS $<TARGET_FILE:${PLUGIN_target}>)
            endif()

            lore_generate_thunk(${HTL} Thunk_manifest_host.cpp ${HTL_src} Thunk_config.conf
                OUT_CALLBACKS_FILE ${LORELEI_THUNK_SOURCES_DIR}/${PROJECT_NAME}/Thunk_callbacks.txt
                EXTRA_ARGS ${_extra_args} ${LORELEI_HOST_TLC_ARGS}
                PRE_INCLUDE_FILES Thunk_procs_desc.h
                DEPENDS Thunk_procs_desc.h Thunk_config_symbols.conf
                ${_options}
            )
        endif()

        add_library(${HTL} SHARED ${HTL_src})

        if(HTL_alias)
            set_target_properties(${HTL} PROPERTIES POST_LINK_ALIAS "${HTL_alias}")
        endif()

        lore_configure_target_common(${HTL})
        lore_configure_host_thunk(${HTL})
        lore_host_thunk_disable_register(${HTL})
        target_link_libraries(${HTL} PUBLIC LoreHostRT)

        if(HTL_extra_includes)
            target_include_directories(${HTL} PRIVATE ${HTL_extra_includes})
        endif()

        if(HTL_extra_links)
            target_link_libraries(${HTL} PRIVATE ${HTL_extra_links})
        endif()

        if(HTL_force_links)
            target_link_options(${HTL} PRIVATE -Wl,--no-as-needed)
            target_link_libraries(${HTL} PRIVATE ${HTL_force_links})
        endif()
    endif()
endmacro()

project(SDL2-2.0)

set(_lib_inc ${LORELEI_HOST_LIBRARIES_DIR}/SDL/include)

set(_GTL ${PROJECT_NAME}_GTL)
set(_HTL ${PROJECT_NAME}_HTL)

set(_GTL_src ${LORELEI_THUNK_SOURCES_DIR}/${PROJECT_NAME}/Thunk_guest.cpp)
set(_HTL_src ${LORELEI_THUNK_SOURCES_DIR}/${PROJECT_NAME}/Thunk_host.cpp)
set(_extra_args -I${_lib_inc})

if(LORELEI_ENABLE_GUEST)
    if(LORELEI_THUNK_GENERATE)
        lore_generate_thunk(${_GTL} Thunk_manifest_guest.cpp ${_GTL_src} Thunk_config.conf
            EXTRA_ARGS ${_extra_args} ${LORELEI_GUEST_TLC_ARGS}
            PRE_INCLUDE_FILES Thunk_procs_desc.h
            DEPENDS Thunk_procs_desc.h Thunk_config_symbols.conf
        )
    endif()

    add_library(${_GTL} SHARED ${_GTL_src})

    set(_GTL_alias libSDL2.so libSDL2-2.0.so.0)
    set_target_properties(${_GTL} PROPERTIES POST_LINK_ALIAS "${_GTL_alias}")

    lore_configure_target_common(${_GTL})
    lore_configure_guest_thunk(${_GTL})
    lore_guest_thunk_disable_register(${_GTL})
    target_link_libraries(${_GTL} PUBLIC LoreGuestRT)

    target_link_libraries(${_GTL} PRIVATE dl)
    target_include_directories(${_GTL} PRIVATE ${_lib_inc})
endif()

if(LORELEI_ENABLE_HOST)
    if(LORELEI_THUNK_GENERATE)
        lore_generate_thunk(${_HTL} Thunk_manifest_host.cpp ${_HTL_src} Thunk_config.conf
            OUT_CALLBACKS_FILE ${LORELEI_THUNK_SOURCES_DIR}/${PROJECT_NAME}/Thunk_callbacks.txt
            EXTRA_ARGS ${_extra_args} ${LORELEI_HOST_TLC_ARGS}
            PRE_INCLUDE_FILES Thunk_procs_desc.h
            DEPENDS Thunk_procs_desc.h Thunk_config_symbols.conf
        )
    endif()

    add_library(${_HTL} SHARED ${_HTL_src})

    lore_configure_target_common(${_HTL})
    lore_configure_host_thunk(${_HTL})
    lore_host_thunk_disable_register(${_HTL})
    target_link_libraries(${_HTL} PUBLIC LoreHostRT)

    target_include_directories(${_HTL} PRIVATE ${_lib_inc})
endif()
include("conf.cmake")

lore_init_buildsystem()

# Project variables
if(NOT DEFINED LORE_THUNK_DATA_DIR)
    set(LORE_THUNK_DATA_DIR ${LORE_BUILD_SHARE_DIR}/lorelei/thunks)
endif()

if(NOT DEFINED LORE_THUNK_SOURCE_DIR)
    set(LORE_THUNK_SOURCE_DIR ${LORE_BUILD_BASE_DIR}/src/thunks)
endif()

# Target for generating all thunks
add_custom_target(TLC_stat_all)
add_custom_target(TLC_generate_all)

add_subdirectory(3rdparty)
add_subdirectory(base)
add_subdirectory(modules)

if(LORE_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(LORE_BUILD_THUNKS)
    add_subdirectory(thunks)
endif()

lore_install()

if(LORE_INSTALL)
    # Install shared files
    include("${LORE_SOURCE_DIR}/share/install.cmake")

    install(DIRECTORY ${LORE_THUNK_DATA_DIR}/
        DESTINATION ${LORE_INSTALL_DATA_DIR}/thunks
    )

    install(DIRECTORY ${LORE_THUNK_SOURCE_DIR}/
        DESTINATION src/thunks
    )
endif()

lore_finish_buildsystem()

include("conf.cmake")

lore_init_buildsystem()

# Project variables
set(LORE_THUNK_DATA_DIR ${LORE_BUILD_SHARE_DIR}/lorelei/thunks)
set(LORE_THUNK_SOURCE_DIR ${LORE_BUILD_BASE_DIR}/src/thunks)

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
endif()

lore_finish_buildsystem()

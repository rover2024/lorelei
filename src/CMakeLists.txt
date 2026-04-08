include("conf.cmake")

lore_init_buildsystem()

# Target for generating all thunks
add_custom_target(thunk_gen_all)

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

lore_finish_buildsystem()
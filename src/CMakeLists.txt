include("conf.cmake")

lore_init_buildsystem()

add_subdirectory(3rdparty)
add_subdirectory(base)
add_subdirectory(modules)

if(LORE_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

lore_install()

lore_finish_buildsystem()

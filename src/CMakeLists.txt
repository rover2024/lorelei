include("conf.cmake")

lore_init_buildsystem()

add_subdirectory(3rdparty)
add_subdirectory(libs)
add_subdirectory(modules)
add_subdirectory(utils)

if(LORE_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if (LORE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

lore_install()

lore_finish_buildsystem()

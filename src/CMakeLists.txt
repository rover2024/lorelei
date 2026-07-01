include("conf.cmake")

lore_init_buildsystem()

qm_add_definition(LORE_CONFIG_THUNK_VARG_MAX ${LORE_CONFIG_THUNK_VARG_MAX})
qm_add_definition(LORE_CONFIG_QEMU_SUPPORT_ADDRESS_SEPARATION CONDITION LORE_CONFIG_QEMU_SUPPORT_ADDRESS_SEPARATION)

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

install(FILES ../LICENSE DESTINATION ${LORE_INSTALL_DATA_DIR})

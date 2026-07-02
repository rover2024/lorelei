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

    # For a self-contained toolchain, bundle Clang's builtin headers next to the tools. Clang's
    # embedded driver looks for them at <exe-dir>/../lib/clang/<version>/include, so a copied install
    # resolves them without a system Clang present. The version subdirectory comes from the install
    # tree itself, so nothing here hard-codes the LLVM version.
    if(LORE_STATIC_LLVM AND LORE_INSTALL)
        find_package(Clang REQUIRED)
        install(DIRECTORY ${LLVM_LIBRARY_DIRS}/clang/
            DESTINATION lib/clang
            FILES_MATCHING PATTERN "*.h" PATTERN "*.modulemap")
    endif()
endif()

if (LORE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

lore_install()

lore_finish_buildsystem()

install(FILES ../LICENSE DESTINATION ${LORE_INSTALL_DATA_DIR})

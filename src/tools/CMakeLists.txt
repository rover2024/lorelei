add_subdirectory(libs)

add_subdirectory(tools)

if (LORE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
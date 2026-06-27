# The TLC tests drive the LoreTLC executable, so they are only built when the tools are.
if(NOT TARGET LoreTLC)
    return()
endif()

add_auto_test(tst_TLC.cpp)
add_dependencies(tst_TLC LoreTLC)

# The test syntax-checks the generated thunk sources in process with Clang.
lore_link_clang(tst_TLC PRIVATE)

# Ship the fixture thunk (Desc.h, Symbols.conf, manifests) next to the test binary.
qm_add_copy_command(tst_TLC
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/TestData/
    DESTINATION tst_TLC_dir/TestData
)

target_compile_definitions(tst_TLC PRIVATE
    LORE_TLC_EXECUTABLE="$<TARGET_FILE:LoreTLC>"
    LORE_TLC_SOURCE_INCLUDE="${LORE_SOURCE_DIR}/include"
    LORE_TLC_BUILD_INCLUDE="${LORE_BUILD_INCLUDE_DIR}"
    LORE_TEST_DATA_DIR="$<TARGET_FILE_DIR:tst_TLC>/tst_TLC_dir/TestData"
)

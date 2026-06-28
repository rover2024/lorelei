add_auto_test(tst_StringExtras.cpp LoreSupport)
add_auto_test(tst_ScopeGuard.cpp)
add_auto_test(tst_ConfigFile.cpp LoreSupport)
add_auto_test(tst_Logging.cpp LoreSupport)
add_auto_test(tst_VarSizeArray.cpp)
add_auto_test(tst_STLTraitExtras.cpp)

# Ship the INI fixtures next to the test binary.
qm_add_copy_command(tst_ConfigFile
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/TestData/
    DESTINATION tst_ConfigFile_dir/TestData
)
target_compile_definitions(tst_ConfigFile PRIVATE
    LORE_TEST_DATA_DIR="$<TARGET_FILE_DIR:tst_ConfigFile>/tst_ConfigFile_dir/TestData"
)

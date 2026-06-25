add_auto_test(StringExtras.cpp LoreSupport)
add_auto_test(ScopeGuard.cpp)
add_auto_test(ConfigFile.cpp LoreSupport)
add_auto_test(Logging.cpp LoreSupport)

# Ship the INI fixtures next to the test binary and point the test at them.
qm_add_copy_command(tst_ConfigFile
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/TestData/
    DESTINATION TestData
)
target_compile_definitions(tst_ConfigFile PRIVATE
    LORE_TEST_DATA_DIR="$<TARGET_FILE_DIR:tst_ConfigFile>/TestData"
)

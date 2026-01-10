set(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG QUIET COMPONENTS unit_test_framework)

if(NOT Boost_FOUND)
    message(WARNING "Boost not found, auto tests will not be built")
    return()
endif()

lore_add_auto_test(tst_VariadicAdaptor.cpp)
target_link_libraries(tst_VariadicAdaptor PRIVATE LoreCore)

if(NOT WIN32)
    lore_add_auto_test(tst_CallbackTrampoline.cpp)
    target_link_libraries(tst_CallbackTrampoline PRIVATE LoreCore)
    target_compile_options(tst_CallbackTrampoline PRIVATE "-ffixed-${LORE_HOST_FIXED_REGISTER}")
endif()
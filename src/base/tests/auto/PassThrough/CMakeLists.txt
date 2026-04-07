lore_add_auto_test(tst_VariadicAdaptor.cpp)
target_link_libraries(tst_VariadicAdaptor PRIVATE LorePassThrough)

# if(NOT WIN32)
#     lore_add_auto_test(tst_CallbackTrampoline.cpp)
#     target_link_libraries(tst_CallbackTrampoline PRIVATE LoreSupport)
#     target_compile_options(tst_CallbackTrampoline PRIVATE "-ffixed-${LORE_HOST_FIXED_REGISTER}")
# endif()
add_auto_test(tst_FunctionTrampoline.cpp LoreDLCall)
add_auto_test(tst_VariadicAdaptor.cpp LoreDLCall)
add_auto_test(tst_VariadicArgDefs.cpp)
add_auto_test(tst_ThunkDatabase.cpp LoreDLCall)

# The trampoline test calls through a generated trampoline that carries its context in the host
# fixed register; the calling code must therefore leave that register untouched, so reserve it with
# -ffixed (the same guarantee any real user of the trampoline / callback macros has to make).
target_compile_options(tst_FunctionTrampoline PRIVATE -ffixed-${LORE_HOST_FIXED_REGISTER})

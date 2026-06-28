add_auto_test(tst_CallbackTrampoline.cpp LoreDLCall)
add_auto_test(tst_FunctionTrampoline.cpp LoreDLCall)
add_auto_test(tst_VariadicAdaptor.cpp LoreDLCall)
add_auto_test(tst_VariadicArgDefs.cpp)

# The trampoline tests call through a generated trampoline that carries its context in the host
# fixed register; the calling code must therefore leave that register untouched, so reserve it with
# -ffixed (the same guarantee any real user of the trampoline / callback macros has to make).
foreach(_t tst_CallbackTrampoline tst_FunctionTrampoline)
    target_compile_options(${_t} PRIVATE -ffixed-${LORE_HOST_FIXED_REGISTER})
endforeach()

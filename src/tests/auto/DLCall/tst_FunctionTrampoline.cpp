// SPDX-License-Identifier: MIT

#include <cstdint>

#include <lorelei/DLCall/Tools/FunctionTrampoline.h>
#include <lorelei/ThunkInterface/Detail/Callback.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

BOOST_AUTO_TEST_SUITE(test_FunctionTrampoline)

typedef int (*Operator)(int a, int b);

static void *last_function;

// Common landing function: the trampoline's shared stub leaves the original function in the
// scratch register before jumping here.
static int operator_thunk(int a, int b) {
    LORE_THUNK_GET_LAST_GCB(fn)

    last_function = fn;
    return ((Operator) fn)(a, b);
}

static int add(int a, int b) {
    return a + b;
}

static int mul(int a, int b) {
    return a * b;
}

BOOST_AUTO_TEST_CASE(routes_to_saved_function) {
    constexpr uintptr_t kMagic = 0xABCDEF;
    auto *table = FunctionTrampolineTable::create(4, (void *) operator_thunk, kMagic);
    BOOST_VERIFY(table != static_cast<FunctionTrampolineTable *>(nullptr));

    table->trampoline[0].saved_function = (void *) add;
    table->trampoline[1].saved_function = (void *) mul;

    last_function = nullptr;
    BOOST_VERIFY(((Operator) table->trampoline[0].thunk_instr)(3, 4) == 7);
    BOOST_VERIFY(last_function == (void *) add);

    last_function = nullptr;
    BOOST_VERIFY(((Operator) table->trampoline[1].thunk_instr)(3, 4) == 12);
    BOOST_VERIFY(last_function == (void *) mul);

    // magic_sign tags each instance so a raw pointer can be recognized as a trampoline.
    BOOST_VERIFY(table->trampoline[0].magic_sign == kMagic);
    BOOST_VERIFY(table->trampoline[1].magic_sign == kMagic);

    FunctionTrampolineTable::destroy(table);
}

BOOST_AUTO_TEST_SUITE_END()

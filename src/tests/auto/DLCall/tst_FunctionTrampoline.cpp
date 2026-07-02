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

// Common handler: each stub calls this and returns; the handler recovers the original function from
// its own return address. Its address is taken to build the stubs, so it is emitted out of line.
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
    BOOST_TEST(table != static_cast<FunctionTrampolineTable *>(nullptr));

    table->trampoline[0].saved_function = (void *) add;
    table->trampoline[1].saved_function = (void *) mul;

    last_function = nullptr;
    BOOST_TEST(((Operator) table->trampoline[0].thunk_instr)(3, 4) == 7);
    BOOST_TEST(last_function == (void *) add);

    last_function = nullptr;
    BOOST_TEST(((Operator) table->trampoline[1].thunk_instr)(3, 4) == 12);
    BOOST_TEST(last_function == (void *) mul);

    // magic_sign tags each instance so a raw pointer can be recognized as a trampoline.
    BOOST_TEST(table->trampoline[0].magic_sign == kMagic);
    BOOST_TEST(table->trampoline[1].magic_sign == kMagic);

    FunctionTrampolineTable::destroy(table);
}

// allocCallbackTrampoline hands out one stub per distinct callback (dedup), routes each to its
// original, and unwrapTrampoline recovers the original from a bare stub pointer.
BOOST_AUTO_TEST_CASE(alloc_dedups_routes_and_unwraps) {
    using namespace lore::thunk;

    auto s_add = allocCallbackTrampoline<operator_thunk>((void *) add);
    auto s_add2 = allocCallbackTrampoline<operator_thunk>((void *) add);
    auto s_mul = allocCallbackTrampoline<operator_thunk>((void *) mul);

    BOOST_TEST(s_add == s_add2); // same input -> same stub
    BOOST_TEST(s_add != s_mul);  // distinct inputs -> distinct stubs

    last_function = nullptr;
    BOOST_TEST(s_add(3, 4) == 7);
    BOOST_TEST(last_function == (void *) add);

    last_function = nullptr;
    BOOST_TEST(s_mul(3, 4) == 12);
    BOOST_TEST(last_function == (void *) mul);

    // A stub reverts to its original; a non-trampoline pointer passes through unchanged.
    BOOST_TEST(unwrapTrampoline((void *) s_add) == (void *) add);
    BOOST_TEST(unwrapTrampoline((void *) add) == (void *) add);
}

BOOST_AUTO_TEST_SUITE_END()

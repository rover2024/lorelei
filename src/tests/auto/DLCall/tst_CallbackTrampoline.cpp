// SPDX-License-Identifier: MIT

#include <lorelei/ThunkInterface/Detail/Callback.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

BOOST_AUTO_TEST_SUITE(test_CallbackTrampoline)

typedef int (*Operator)(int a, int b);

static void *last_callback;

// Common landing function: recovers the original callback from the scratch register
// (loaded by the trampoline's shared stub) and forwards to it.
static int operator_thunk(int a, int b) {
    LORE_THUNK_GET_LAST_GCB(callback)

    last_callback = callback;
    return ((Operator) callback)(a, b);
}

static int add(int a, int b) {
    return a + b;
}

static int sub(int a, int b) {
    return a - b;
}

BOOST_AUTO_TEST_CASE(routes_to_saved_callback) {
    auto add_thunk = lore::thunk::allocCallbackTrampoline<operator_thunk>((void *) add);
    last_callback = nullptr;
    BOOST_VERIFY(add_thunk(1, 2) == 3);
    BOOST_VERIFY(last_callback == (void *) add);

    auto sub_thunk = lore::thunk::allocCallbackTrampoline<operator_thunk>((void *) sub);
    last_callback = nullptr;
    BOOST_VERIFY(sub_thunk(1, 2) == -1);
    BOOST_VERIFY(last_callback == (void *) sub);
}

BOOST_AUTO_TEST_CASE(dedups_by_callback) {
    // The same callback maps to the same trampoline instance.
    auto a = lore::thunk::allocCallbackTrampoline<operator_thunk>((void *) add);
    auto b = lore::thunk::allocCallbackTrampoline<operator_thunk>((void *) add);
    BOOST_VERIFY((void *) a == (void *) b);

    // A null callback yields a null trampoline.
    auto n = lore::thunk::allocCallbackTrampoline<operator_thunk>(nullptr);
    BOOST_VERIFY((void *) n == (void *) nullptr);
}

BOOST_AUTO_TEST_SUITE_END()

// TODO: #include <lorelei/TLCMeta/ManifestCallbackDefs.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

BOOST_AUTO_TEST_SUITE(test_CallbackTrampoline)

typedef int (*Operator)(int a, int b);

static void *last_callback;

static int operator_thunk(int a, int b) {
    LORETHUNK_GET_LAST_GCB(callback)

    last_callback = callback;
    return ((Operator) callback)(a, b);
}

static int add(int a, int b) {
    return a + b;
}

static int sub(int a, int b) {
    return a - b;
}

BOOST_AUTO_TEST_CASE(test_trampoline) {
    int result;

    auto add_thunk = lorethunk::allocCallbackTrampoline<operator_thunk>((void *) add);
    result = add_thunk(1, 2);
    BOOST_VERIFY(last_callback == (void *) add);
    BOOST_VERIFY(result == 3);

    auto sub_thunk = lorethunk::allocCallbackTrampoline<operator_thunk>((void *) sub);
    result = sub_thunk(1, 2);
    BOOST_VERIFY(last_callback == (void *) sub);
    BOOST_VERIFY(result == -1);
}

BOOST_AUTO_TEST_SUITE_END()

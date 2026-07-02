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

// Common handler: each stub tail-branches here (via the shim) after the shim parks the identity in
// thread_last_callback, and the handler reads it back to recover the original function.
static int operator_thunk(int a, int b) {
    LORE_THUNK_GET_LAST_GCB(fn)

    last_function = fn;
    return ((Operator) fn)(a, b);
}

// Hide a pointer's provenance from the optimizer. unwrapTrampoline reads the magic word that sits
// just before a stub, so probing a bare function pointer reads before it. At -O2 GCC otherwise sees
// this as indexing before a compile-time-known function object and rejects it. Real callbacks are
// runtime pointers where this analysis cannot fire, so this only matters for the test's own probes.
static void *opaque(void *p) {
    asm volatile("" : "+r"(p));
    return p;
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

    // A stub reverts to its original, and a non-trampoline pointer passes through unchanged.
    BOOST_TEST(unwrapTrampoline((void *) s_add) == (void *) add);
    BOOST_TEST(unwrapTrampoline(opaque((void *) add)) == (void *) add);
}

// Capture the stack pointer the handler is entered with. The stub tail-branches to the handler (via
// the shim), never opening a frame, so the handler runs at the same ABI stack alignment a direct call
// would. This probe compares the two: a stub that instead reached the handler through a call would
// shift the low bits, which is exactly what would make a handler's aligned SSE spills fault.
static uintptr_t observed_sp;
static int capture_sp(int, int) {
    uintptr_t sp;
#if defined(__x86_64__)
    asm volatile("mov %%rsp, %0" : "=r"(sp));
#elif defined(__aarch64__)
    asm volatile("mov %0, sp" : "=r"(sp));
#elif defined(__riscv)
    asm volatile("mv %0, sp" : "=r"(sp));
#else
    sp = (uintptr_t) __builtin_frame_address(0);
#endif
    observed_sp = sp;
    return 0;
}

BOOST_AUTO_TEST_CASE(handler_enters_at_abi_stack_alignment) {
    using namespace lore::thunk;
    // Direct call, through an opaque pointer so it is a real call and not inlined into the test.
    ((int (*) (int, int)) opaque((void *) capture_sp))(0, 0);
    uintptr_t direct = observed_sp;
    // The same function reached through a trampoline stub.
    auto stub = allocCallbackTrampoline<capture_sp>((void *) capture_sp);
    stub(0, 0);
    uintptr_t via_stub = observed_sp;

    BOOST_TEST((direct & 0xF) == (via_stub & 0xF));
}

// Forward-argument and return-value integrity, including stack-passed arguments. Ten integer plus two
// floating arguments overflow the integer register budget on every target (six on x86_64, eight on
// aarch64/riscv64), so a9/a10 (and a7/a8 on x86_64) arrive on the stack. Because the stub tail-
// branches, it leaves the caller's stack frame in place, so those must reach the handler intact.
static long int_log[10];
static double fp_log[2];
static long forward_args(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8,
                         long a9, long a10, double f1, double f2) {
    int_log[0] = a1;
    int_log[1] = a2;
    int_log[2] = a3;
    int_log[3] = a4;
    int_log[4] = a5;
    int_log[5] = a6;
    int_log[6] = a7;
    int_log[7] = a8;
    int_log[8] = a9;
    int_log[9] = a10;
    fp_log[0] = f1;
    fp_log[1] = f2;
    return a1 + a10;
}

static double ratio(int a, int b) {
    return (double) a / (double) b;
}

BOOST_AUTO_TEST_CASE(forwards_register_and_stack_arguments_and_return) {
    using namespace lore::thunk;

    auto stub = allocCallbackTrampoline<forward_args>((void *) forward_args);
    long ret = stub(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1.5, 2.5);
    for (long i = 0; i < 10; i++) {
        BOOST_TEST(int_log[i] == i + 1); // register AND stack-passed integers
    }
    BOOST_TEST(fp_log[0] == 1.5);
    BOOST_TEST(fp_log[1] == 2.5);
    BOOST_TEST(ret == 11);

    // A floating return goes back in the FP return register (xmm0 / d0 / fa0), which the stub must
    // also leave alone.
    auto stub_fp = allocCallbackTrampoline<ratio>((void *) ratio);
    BOOST_TEST(stub_fp(3, 4) == 0.75);
}

BOOST_AUTO_TEST_SUITE_END()

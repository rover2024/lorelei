// Guest test program for the ThunkExample thunk. It is an ordinary x86_64 program that calls the
// le_* functions; linked against the guest thunk (libThunkExample.so) and run under the patched
// QEMU with the dlcall plugin, every call is carried across to the host implementation. See the
// run_manual_tlc target and README in this directory.

#include "ThunkExample.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static int failures = 0;

// Check a condition and report the function's result on its own line.
#define EXPECT(label, cond)                                                                        \
    do {                                                                                           \
        if (cond) {                                                                                \
            printf("  %-14s OK\n", label);                                                         \
        } else {                                                                                   \
            printf("  %-14s FAIL (%s)\n", label, #cond);                                           \
            failures++;                                                                            \
        }                                                                                          \
    } while (0)

// For the void emit functions there is no return value to check: they print on the host side, so a
// completed call is the success.
#define PASS(label) printf("  %-14s OK\n", label)

// A guest callback that the host's qsort / bsearch calls back into.
static int compare_int(const void *a, const void *b) {
    int x = *(const int *) a;
    int y = *(const int *) b;
    return (x > y) - (x < y);
}

// The nested-callback stress test. le_visit walks a three-level tree and calls a callback in every
// set node; this records the node tags it saw, so we can check the thunk wrapped a guest callback at
// every depth and on both reach kinds (direct field and pointer), and honored the null guards.
static unsigned visited_mask;
static void visit_cb(int tag) {
    visited_mask |= 1u << tag;
}

// The set/get round-trip handler. le_set_handler registers it; le_call_handler has the host call it
// (forward), and le_get_handler hands it back through a pointer-to-callback out parameter so the
// guest can call it too. Both invocations must reach this one function.
static int handler_total;
static void my_handler(int x) {
    handler_total += x;
}

// Wrappers that build a va_list for the va_list-form functions.
static int call_vprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = le_vprintf(fmt, ap);
    va_end(ap);
    return ret;
}

static int call_vsscanf(const char *str, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = le_vsscanf(str, fmt, ap);
    va_end(ap);
    return ret;
}

static void call_vemit(int channel, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    le_vemit(channel, fmt, ap);
    va_end(ap);
}

static void call_vemit_attr(int channel, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    le_vemit_attr(channel, fmt, ap);
    va_end(ap);
}

int main(void) {
    printf("ThunkExample guest test\n");

    // printf family: variadic argument marshalling, run by the host's vprintf.
    int n = le_printf("le_printf: %s = %d\n", "answer", 42);
    EXPECT("le_printf:", n > 0);

    int vn = call_vprintf("le_vprintf: %s = %d\n", "answer", 42);
    EXPECT("le_vprintf:", vn > 0);

    // scanf family: output pointers written back across the boundary.
    int year = 0;
    int got = le_sscanf("year=2026", "year=%d", &year);
    EXPECT("le_sscanf:", got == 1 && year == 2026);

    int a = 0, b = 0;
    int vgot = call_vsscanf("3 4", "%d %d", &a, &b);
    EXPECT("le_vsscanf:", vgot == 2 && a == 3 && b == 4);

    // emit family: same printf shape but an obscure name, recognised by descriptor / attribute.
    // These return void and print "[chN] ..." on the host's stderr, so a completed call is success.
    le_emit(1, "le_emit: %d\n", 7);
    PASS("le_emit:");
    le_emit_attr(2, "le_emit_attr: %d\n", 8);
    PASS("le_emit_attr:");
    call_vemit(3, "le_vemit: %d\n", 9);
    PASS("le_vemit:");
    call_vemit_attr(4, "le_vemit_attr: %d\n", 10);
    PASS("le_vemit_attr:");

    // Callbacks: the host qsort / bsearch call compare_int back in the guest (reentry).
    int arr[] = {5, 3, 9, 1, 7};
    size_t count = sizeof(arr) / sizeof(arr[0]);
    le_qsort(arr, count, sizeof(int), compare_int);
    EXPECT("le_qsort:", arr[0] == 1 && arr[count - 1] == 9);

    int key = 7;
    int *found = (int *) le_bsearch(&key, arr, count, sizeof(int), compare_int);
    EXPECT("le_bsearch:", found != NULL && *found == 7);

    // long double: round-trips through the type filter.
    long double mixed = le_mix(1.0L, 3.0L);
    EXPECT("le_mix:", mixed == 2.0L);

    // Nested callbacks: a three-level tree with a callback in every node, each level reached both as
    // a direct struct field and through a pointer. Tags 1..7 identify the positions:
    //   1 root              2 root.child            3 root.child.child     4 root.child.child_ptr
    //   5 root.child_ptr    6 root.child_ptr.child  7 root.child_ptr.child_ptr
    struct le_node3 vcc = {3, visit_cb};
    struct le_node3 vcp = {4, visit_cb};
    struct le_node3 pcc = {6, visit_cb};
    struct le_node3 pcp = {7, visit_cb};
    struct le_node2 v = {2, visit_cb, vcc, &vcp};
    struct le_node2 p = {5, visit_cb, pcc, &pcp};
    struct le_node1 root = {1, visit_cb, v, &p};

    // Everything present: all seven callbacks fire.
    visited_mask = 0;
    le_visit(&root);
    EXPECT("le_visit full:", visited_mask == 0xFEu);

    // Null guards: drop both pointer children and one callback. Only the direct-field chain whose
    // callbacks are set still fires (tag 1 root, tag 3 root.child.child); tag 2 is a null callback.
    visited_mask = 0;
    root.child_ptr = NULL;       // removes 5, 6, 7
    root.child.child_ptr = NULL; // removes 4
    root.child.cb = NULL;        // removes 2 (null callback, host skips it)
    le_visit(&root);
    EXPECT("le_visit nulls:", visited_mask == ((1u << 1) | (1u << 3)));

    // set / get round-trip: register my_handler, have the host call it, then get it back through the
    // out parameter and call it from the guest. Both directions must reach my_handler, and get must
    // return the original guest function (the thunk undoes its own trampoline).
    handler_total = 0;
    le_set_handler(my_handler);
    le_call_handler(3); // host calls the stored handler -> reenters the guest
    EXPECT("le_call_handler:", handler_total == 3);

    le_handler_fn handler = NULL;
    le_get_handler(&handler); // host returns the handler through the pointer-to-callback out param
    EXPECT("le_get_handler:", handler == my_handler);
    if (handler) {
        handler(5); // guest calls the retrieved handler directly
    }
    EXPECT("le_get_handler call:", handler_total == 8);

    if (failures == 0) {
        printf("ThunkExample guest test: OK\n");
        return 0;
    }
    fprintf(stderr, "ThunkExample guest test: %d failure(s)\n", failures);
    return 1;
}

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

    if (failures == 0) {
        printf("ThunkExample guest test: OK\n");
        return 0;
    }
    fprintf(stderr, "ThunkExample guest test: %d failure(s)\n", failures);
    return 1;
}

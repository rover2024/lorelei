// Guest test program for the ThunkExample thunk. It is an ordinary x86_64 program that calls the
// le_* functions; linked against the guest thunk (libThunkExample.so) and run under the patched
// QEMU with the dlcall plugin, every call is carried across to the host implementation. See
// run.sh.

#include "ThunkExample.h"

#include <stdio.h>
#include <stdlib.h>

static int failures = 0;

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "FAIL: %s\n", #cond);                                                  \
            failures++;                                                                            \
        }                                                                                          \
    } while (0)

// A guest callback that the host's qsort / bsearch calls back into.
static int compare_int(const void *a, const void *b) {
    int x = *(const int *) a;
    int y = *(const int *) b;
    return (x > y) - (x < y);
}

int main(void) {
    // printf family: variadic argument marshalling, run by the host's vprintf.
    int n = le_printf("le_printf: %s = %d\n", "answer", 42);
    CHECK(n > 0);

    // The emit family has the same shape but an obscure name: recognised by descriptor / attribute.
    le_emit(1, "le_emit: %d\n", 7);
    le_emit_attr(2, "le_emit_attr: %d\n", 8);

    // scanf family: output pointers written across the boundary.
    int year = 0;
    int got = le_sscanf("year=2026", "year=%d", &year);
    CHECK(got == 1 && year == 2026);

    // Callbacks: the host qsort / bsearch call compare_int back in the guest (reentry).
    int arr[] = {5, 3, 9, 1, 7};
    size_t count = sizeof(arr) / sizeof(arr[0]);
    le_qsort(arr, count, sizeof(int), compare_int);
    CHECK(arr[0] == 1 && arr[count - 1] == 9);

    int key = 7;
    int *found = (int *) le_bsearch(&key, arr, count, sizeof(int), compare_int);
    CHECK(found != NULL && *found == 7);

    // long double: round-trips through the type filter.
    long double mixed = le_mix(1.0L, 3.0L);
    CHECK(mixed == 2.0L);

    if (failures == 0) {
        printf("ThunkExample guest test: OK\n");
        return 0;
    }
    fprintf(stderr, "ThunkExample guest test: %d failure(s)\n", failures);
    return 1;
}

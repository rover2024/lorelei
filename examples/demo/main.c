#include "demo.h"

#include <stdio.h>

static int compare_ints(const void *lhs, const void *rhs) {
    const int a = *(const int *)lhs;
    const int b = *(const int *)rhs;

    return (a > b) - (a < b);
}

static int demo_vprintf_wrapper(const char *format, ...) {
    va_list args;
    int written;

    va_start(args, format);
    written = demo_vprintf(format, args);
    va_end(args);

    return written;
}

int main(void) {
    int values[] = {5, 1, 4, 2, 3};
    const size_t count = sizeof(values) / sizeof(values[0]);

    demo_puts("demo_puts: Hello from the guest program");
    demo_printf("demo_printf: %s %d + %d = %d\n", "checking", 20, 22,
                 demo_add(20, 22));
    demo_vprintf_wrapper("demo_vprintf: %s %d\n", "checking", 7);

    demo_qsort(values, count, sizeof(values[0]), compare_ints);

    demo_printf("demo_qsort:");
    for (size_t i = 0; i < count; ++i) {
        demo_printf(" %d", values[i]);
    }
    demo_printf("\n");

    if (demo_add(20, 22) != 42) {
        fprintf(stderr, "demo_add failed\n");
        return 1;
    }

    for (size_t i = 0; i < count; ++i) {
        if (values[i] != (int)i + 1) {
            fprintf(stderr, "demo_qsort failed at index %zu\n", i);
            return 1;
        }
    }

    return 0;
}

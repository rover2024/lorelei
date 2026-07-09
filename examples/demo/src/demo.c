#include "demo.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int demo_add(int lhs, int rhs) {
    return lhs + rhs;
}

void demo_puts(const char *message) {
    puts(message);
}

int demo_printf(const char *format, ...) {
    va_list args;
    int written;

    va_start(args, format);
    written = demo_vprintf(format, args);
    va_end(args);

    return written;
}

int demo_vprintf(const char *format, va_list args) {
    int written = vprintf(format, args);
    return written;
}

void demo_qsort(void *base, size_t nmemb, size_t size, demo_compare_fn compar) {
    qsort(base, nmemb, size, compar);
}

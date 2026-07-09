#pragma once

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*demo_compare_fn)(const void *, const void *);

int demo_add(int lhs, int rhs);
void demo_puts(const char *message);
int demo_printf(const char *format, ...) __attribute__((format(printf, 1, 2)));
int demo_vprintf(const char *format, va_list args);
void demo_qsort(void *base, size_t nmemb, size_t size, demo_compare_fn compar);

#ifdef __cplusplus
}
#endif

#include "ThunkExample.h"

#include <cstdio>
#include <cstdlib>

// Host-side implementation of the example library. Each function just forwards to the real libc
// routine, so the interesting part is purely the thunk that carries the call across the boundary.
// Built into the host thunk by the manual end-to-end test (src/tests/manual/TLC).

extern "C" {

    int le_printf(const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        int ret = std::vprintf(fmt, ap);
        va_end(ap);
        return ret;
    }

    int le_vprintf(const char *fmt, va_list ap) {
        return std::vprintf(fmt, ap);
    }

    int le_sscanf(const char *str, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        int ret = std::vsscanf(str, fmt, ap);
        va_end(ap);
        return ret;
    }

    int le_vsscanf(const char *str, const char *fmt, va_list ap) {
        return std::vsscanf(str, fmt, ap);
    }

    void le_qsort(void *base, size_t nmemb, size_t size, le_compare_fn cmp) {
        std::qsort(base, nmemb, size, cmp);
    }

    void *le_bsearch(const void *key, const void *base, size_t nmemb, size_t size,
                     le_compare_fn cmp) {
        return std::bsearch(key, base, nmemb, size, cmp);
    }

    void le_emit(int channel, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        std::fprintf(stderr, "[ch%d] ", channel);
        std::vfprintf(stderr, fmt, ap);
        va_end(ap);
    }

    void le_emit_attr(int channel, const char *fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        std::fprintf(stderr, "[ch%d] ", channel);
        std::vfprintf(stderr, fmt, ap);
        va_end(ap);
    }

    void le_vemit(int channel, const char *fmt, va_list ap) {
        std::fprintf(stderr, "[ch%d] ", channel);
        std::vfprintf(stderr, fmt, ap);
    }

    void le_vemit_attr(int channel, const char *fmt, va_list ap) {
        std::fprintf(stderr, "[ch%d] ", channel);
        std::vfprintf(stderr, fmt, ap);
    }

    long double le_mix(long double a, long double b) {
        return (a + b) / 2.0L;
    }

}

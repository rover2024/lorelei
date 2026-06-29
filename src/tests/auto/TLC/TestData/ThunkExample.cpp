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

    static void le_visit_node3(struct le_node3 *n) {
        if (n && n->cb) {
            n->cb(n->tag);
        }
    }

    static void le_visit_node2(struct le_node2 *n) {
        if (!n) {
            return;
        }
        if (n->cb) {
            n->cb(n->tag);
        }
        le_visit_node3(&n->child);
        le_visit_node3(n->child_ptr);
    }

    void le_visit(struct le_node1 *root) {
        if (!root) {
            return;
        }
        if (root->cb) {
            root->cb(root->tag);
        }
        le_visit_node2(&root->child);
        le_visit_node2(root->child_ptr);
    }

    static le_handler_fn le_stored_handler = nullptr;

    void le_set_handler(le_handler_fn fn) {
        le_stored_handler = fn;
    }

    void le_call_handler(int x) {
        if (le_stored_handler) {
            le_stored_handler(x);
        }
    }

    void le_get_handler(le_handler_fn *out) {
        *out = le_stored_handler;
    }

}

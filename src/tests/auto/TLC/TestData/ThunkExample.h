#ifndef LORE_TLC_TEST_THUNKEXAMPLE_H
#define LORE_TLC_TEST_THUNKEXAMPLE_H

// Usable from both C++ (TLC parses Desc.h, the manifests compile it) and C (the manual guest
// program in src/tests/manual/TLC includes it directly).
#include <stddef.h>
#include <stdarg.h>

// ThunkExample is a small worked example of a thunk library, used both as the TLC test fixture and
// as the manual end-to-end guest test (see src/tests/manual/TLC). Each function wraps a libc
// routine (in the style of the qemu-passthrough-test demos, where my_qsort forwards to the host
// qsort), so a guest that calls these through the generated thunk runs the real work on the host.
// The set is chosen to exercise every TLC feature a real library tends to need:
//
//   le_printf / le_vprintf     a printf-style variadic (and va_list) function
//   le_sscanf / le_vsscanf     a scanf-style function whose format is not the first argument
//   le_qsort  / le_bsearch     functions that take a callback the host calls back into the guest
//   le_emit*  / le_vemit*      printf-style functions whose names do not reveal it, covering the
//                              full matrix of {`...`, va_list} x {has format attribute, none}
//   le_mix                     a function that takes and returns long double

#ifdef __cplusplus
extern "C" {
#endif

    /// A comparator passed to le_qsort / le_bsearch. The host calls it back into guest code.
    typedef int (*le_compare_fn)(const void *a, const void *b);

    /// printf-style: the format string is the first argument, so the thunk infers the indices.
    int le_printf(const char *fmt, ...);

    /// The va_list form of le_printf.
    int le_vprintf(const char *fmt, va_list ap);

    /// scanf-style: the format string is argument 1 (the source string is argument 0).
    int le_sscanf(const char *str, const char *fmt, ...);

    /// The va_list form of le_sscanf.
    int le_vsscanf(const char *str, const char *fmt, va_list ap);

    /// Sorts \a base in place using \a cmp, which the host calls back into the guest.
    void le_qsort(void *base, size_t nmemb, size_t size, le_compare_fn cmp);

    /// Binary-searches \a base for \a key using \a cmp.
    void *le_bsearch(const void *key, const void *base, size_t nmemb, size_t size,
                     le_compare_fn cmp);

    // The four le_emit* / le_vemit* functions all have the same printf-style shape and an obscure
    // name, so neither the name nor the signature alone tells the builder they are printf-like.
    // What differs is how each is recognised: by its format attribute, or (lacking one) by an
    // explicit descriptor in Desc.h.

    /// `...` form, no format attribute: needs an explicit pass::printf descriptor.
    void le_emit(int channel, const char *fmt, ...);

    /// `...` form with a printf format attribute: auto-detected, no descriptor.
    void le_emit_attr(int channel, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

    /// va_list form, no format attribute (like SDL_LogMessageV): needs an explicit pass::vprintf
    /// descriptor.
    void le_vemit(int channel, const char *fmt, va_list ap);

    /// va_list form with a printf format attribute (firstToCheck = 0 marks the va_list form, as
    /// glibc does for vprintf): auto-detected, no descriptor.
    void le_vemit_attr(int channel, const char *fmt, va_list ap) __attribute__((format(printf, 2, 0)));

    /// Takes and returns long double. The guest passes the x86 80-bit extended representation; on
    /// a host whose long double differs, a type filter converts it on the way in and out.
    long double le_mix(long double a, long double b);

#ifdef __cplusplus
}
#endif

#endif // LORE_TLC_TEST_THUNKEXAMPLE_H

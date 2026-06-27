#pragma once

#include <lorelei/ThunkInterface/Proc.h>
#include <lorelei/ThunkInterface/PassTags.h>

// A small self-contained "library" surface used to drive LoreTLC in the tests. The functions are
// only declared (TLC parses declarations), each chosen to exercise one detection path.
extern "C" {

    // A plain function: no special handling.
    int test_add(int a, int b);

    // printf-style whose name ends in "printf": auto-detected by name.
    int test_printf(const char *fmt, ...);

    // printf-style with an obscure name and no format attribute: needs a descriptor.
    void test_emit(int channel, const char *fmt, ...);

    // printf-style with an obscure name but a format attribute: auto-detected by the attribute.
    void test_emit_attr(int channel, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

    // A comparator the host calls back into the guest.
    typedef int (*test_compare_fn)(const void *a, const void *b);
    void test_qsort(void *base, unsigned long nmemb, unsigned long size, test_compare_fn cmp);

}

namespace lore::thunk {

    // test_emit has neither a telling name nor a format attribute, so it is tagged by hand.
    template <>
    struct ProcFnDesc<::test_emit> {
        _DESC pass::printf<2, 3> builder_pass = {};
    };

}

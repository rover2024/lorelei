// SPDX-License-Identifier: MIT

#include <cstdio>
#include <cstdint>
#include <string_view>

#include <lorelei/DLCall/Tools/VariadicAdaptor.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

BOOST_AUTO_TEST_SUITE(test_VariadicAdaptor)

static int forward_to_sxxxf(VariadicAdaptor::FormatStyle style, void *func, char *buffer,
                            const char *fmt, ...) {
    CVargEntry vargs[20];

    // Extract variadic arguments
    {
        va_list ap;
        va_start(ap, fmt);
        VariadicAdaptor::extract(style, fmt, ap, vargs);
        va_end(ap);
    }

    // Construct fixed arguments
    CVargEntry args1[] = {
        CVargGet(buffer),
        CVargGet(fmt),
    };
    CVargEntry vret;
    vret.type = CVargTypeID(int);

    // Call
    VariadicAdaptor::call(func, sizeof(args1) / sizeof(args1[0]), args1, -1, vargs, &vret);

    return CVargValue(int, vret);
}

static int forward_to_vsxxxf(VariadicAdaptor::FormatStyle style, void *func, char *buffer,
                             const char *fmt, ...) {
    CVargEntry vargs[20];

    // Extract variadic arguments
    {
        va_list ap;
        va_start(ap, fmt);
        VariadicAdaptor::extract(style, fmt, ap, vargs);
        va_end(ap);
    }

    // Construct fixed arguments
    CVargEntry args1[] = {
        CVargGet(buffer),
        CVargGet(fmt),
    };
    CVargEntry vret;
    vret.type = CVargTypeID(int);

    // Call
    VariadicAdaptor::vcall(func, sizeof(args1) / sizeof(args1[0]), args1, -1, vargs, &vret);

    return CVargValue(int, vret);
}

// Run a printf format through both the direct (sprintf) and the va_list (vsprintf) paths, checking
// the formatted text and the return value against the real libc call. The first variadic argument is
// the format string. Exercising real conversions is what catches argument-marshalling / ABI faults.
#define CHECK_PRINTF(...)                                                                           \
    do {                                                                                            \
        char _e[2048], _a[2048];                                                                    \
        int _er = sprintf(_e, __VA_ARGS__);                                                         \
        int _ar = forward_to_sxxxf(VariadicAdaptor::PrintF, (void *) sprintf, _a, __VA_ARGS__);     \
        BOOST_TEST(_ar == _er);                                                                     \
        BOOST_TEST(std::string_view(_a) == std::string_view(_e));                                   \
        int _vr = forward_to_vsxxxf(VariadicAdaptor::PrintF, (void *) vsprintf, _a, __VA_ARGS__);   \
        BOOST_TEST(_vr == _er);                                                                      \
        BOOST_TEST(std::string_view(_a) == std::string_view(_e));                                   \
    } while (0)

BOOST_AUTO_TEST_CASE(test_sprintf) {
    const char *fmt = "Float: %f, Int: %d, Str: %s";
    float arg1 = 1145.14;
    int arg2 = 1919180;
    const char *arg3 = "hello";

    char expect_buffer[1024];
    char actual_buffer[1024];

    int expect_ret;
    int actual_ret;

    expect_ret = sprintf(expect_buffer, fmt, arg1, arg2, arg3);

    // sprintf
    actual_ret = forward_to_sxxxf(VariadicAdaptor::PrintF, (void *) sprintf, actual_buffer, fmt,
                                  arg1, arg2, arg3);

    BOOST_TEST(actual_ret == expect_ret);
    BOOST_TEST(std::string_view(actual_buffer) == std::string_view(expect_buffer));

    // vsprintf
    actual_ret = forward_to_vsxxxf(VariadicAdaptor::PrintF, (void *) vsprintf, actual_buffer, fmt,
                                   arg1, arg2, arg3);
    BOOST_TEST(actual_ret == expect_ret);
    BOOST_TEST(std::string_view(actual_buffer) == std::string_view(expect_buffer));
}

BOOST_AUTO_TEST_CASE(test_sscanf) {
    const char *fmt = "Float: %f, Int: %d, Str: %s";
    const char *input = "Float: 1145.14, Int: 1919180, Str: hello";

    float expect_arg1;
    int expect_arg2;
    char expect_arg3[1024];

    float actual_arg1;
    int actual_arg2;
    char actual_arg3[1024];

    int expect_ret;
    int actual_ret;

    expect_ret = sscanf(input, fmt, &expect_arg1, &expect_arg2, &expect_arg3[0]);

    // scanf
    actual_ret = forward_to_sxxxf(VariadicAdaptor::ScanF, (void *) sscanf, (char *) input, fmt,
                                  &actual_arg1, &actual_arg2, actual_arg3);
    BOOST_TEST(actual_ret == expect_ret);
    BOOST_TEST(actual_arg1 == expect_arg1);
    BOOST_TEST(actual_arg2 == expect_arg2);
    BOOST_TEST(std::string_view(actual_arg3) == std::string_view(expect_arg3));

    // vscanf
    actual_ret = forward_to_vsxxxf(VariadicAdaptor::ScanF, (void *) vsscanf, (char *) input, fmt,
                                   &actual_arg1, &actual_arg2, actual_arg3);
    BOOST_TEST(actual_ret == expect_ret);
    BOOST_TEST(actual_arg1 == expect_arg1);
    BOOST_TEST(actual_arg2 == expect_arg2);
    BOOST_TEST(std::string_view(actual_arg3) == std::string_view(expect_arg3));
}

static unsigned long long box64_mix(int a, unsigned long b, long long c, float d, double e,
                                    void *p) {
    return static_cast<unsigned long long>(a) + static_cast<unsigned long long>(b) +
           static_cast<unsigned long long>(c) + static_cast<unsigned long long>(d) +
           static_cast<unsigned long long>(e) +
           (reinterpret_cast<std::uintptr_t>(p) & 0xFFULL);
}

static int box64_effect = 0;

static void box64_side_effect(int a, int b) {
    box64_effect = a * 10 + b;
}

BOOST_AUTO_TEST_CASE(test_callFormatBox64) {
    int a = 7;
    unsigned long b = 9;
    long long c = 11;
    float d = 13.0f;
    double e = 15.0;
    void *p = reinterpret_cast<void *>(0x1234);
    void *args[] = {&a, &b, &c, &d, &e, &p};

    unsigned long long expected = box64_mix(a, b, c, d, e, p);
    unsigned long long actual = 0;
    VariadicAdaptor::callFormatBox64(reinterpret_cast<void *>(box64_mix), "U_iuLfFp", args,
                                     &actual);
    BOOST_TEST(actual == expected);

    int x = 3;
    int y = 5;
    void *void_args[] = {&x, &y};
    box64_effect = 0;
    VariadicAdaptor::callFormatBox64(reinterpret_cast<void *>(box64_side_effect), "v_ii",
                                     void_args, nullptr);
    BOOST_TEST(box64_effect == 35);
}

BOOST_AUTO_TEST_CASE(test_printf_floating) {
    // Several floating-point values back to back. On ABIs where variadic FP shares the integer
    // argument area (riscv64), a half-width or wrongly-placed float misaligns everything after it.
    CHECK_PRINTF("%f", 3.14159);
    CHECK_PRINTF("%f %f %f", 1.5, 2.5, 3.5);
    CHECK_PRINTF("%f|%f|%f|%f|%f", -1.0, 2.0e10, -3.5e-4, 0.0, 1234567.891);
    CHECK_PRINTF("%g %e %f", 0.0001, 123456.789, 42.0);
    CHECK_PRINTF("%.3f %.0f %10.2f", 2.71828, 99.9, 3.5);
}

BOOST_AUTO_TEST_CASE(test_printf_float_int_interleaved) {
    // Floats interleaved with non-floats: the ordering that exposed the riscv64 register mismatch.
    CHECK_PRINTF("%f %d %f %d %s", 1.5, 10, 2.5, 20, "tail");
    CHECK_PRINTF("%d %f %d %f %d %f", 1, 1.5, 2, 2.5, 3, 3.5);
    CHECK_PRINTF("%s %f %s %d %s", "a", 9.0, "b", 7, "c");
}

BOOST_AUTO_TEST_CASE(test_printf_integers) {
    CHECK_PRINTF("%d %u %ld %lu %lld %llu", -7, 8u, -9L, 10UL, -11LL, 12ULL);
    CHECK_PRINTF("%c%c%c %x %X %o", 'a', 'b', 'c', 0xdead, 0xBEEF, 0755);
    CHECK_PRINTF("%d %ld %lld", -2147483647 - 1, -9000000000L, -1234567890123LL);
    CHECK_PRINTF("%llu %lu", 18446744073709551615ULL, 4000000000UL);
}

BOOST_AUTO_TEST_CASE(test_printf_strings_pointers) {
    CHECK_PRINTF("%s/%s/%s", "one", "two", "three");
    CHECK_PRINTF("[%10s][%-10s][%.2s]", "pad", "left", "trunc");
    int x = 0;
    CHECK_PRINTF("%p %p", (void *) &x, (void *) nullptr);
}

BOOST_AUTO_TEST_CASE(test_printf_many_args) {
    // Enough arguments to spill past the argument registers onto the stack, with and without floats.
    CHECK_PRINTF("%d %d %d %d %d %d %d %d %d %d %d %d", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    CHECK_PRINTF("%f %d %f %d %f %d %f %d %f %d", 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5);
}

BOOST_AUTO_TEST_CASE(test_printf_all_kinds) {
    CHECK_PRINTF("%c %d %u %ld %lld %f %g %s %p %x", 'Q', -5, 6u, 100000L, (1LL << 40), 3.14,
                 2.71828, "mixed", (void *) 0xABCD, 0xFF);
}

BOOST_AUTO_TEST_CASE(test_sscanf_mixed) {
    const char *fmt = "%d %f %ld %s %lf";
    const char *input = "42 3.5 1000000 word 2.71828";
    int e_i, a_i;
    float e_f, a_f;
    long e_l, a_l;
    char e_s[64], a_s[64];
    double e_d, a_d;
    int er = sscanf(input, fmt, &e_i, &e_f, &e_l, e_s, &e_d);
    int ar = forward_to_sxxxf(VariadicAdaptor::ScanF, (void *) sscanf, (char *) input, fmt, &a_i,
                              &a_f, &a_l, a_s, &a_d);
    BOOST_TEST(ar == er);
    BOOST_TEST(a_i == e_i);
    BOOST_TEST(a_f == e_f);
    BOOST_TEST(a_l == e_l);
    BOOST_TEST(std::string_view(a_s) == std::string_view(e_s));
    BOOST_TEST(a_d == e_d);
}

BOOST_AUTO_TEST_SUITE_END()

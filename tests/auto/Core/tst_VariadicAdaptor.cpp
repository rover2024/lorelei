#include <cstdio>

#include <lorelei/Core/ThunkTools/VariadicAdaptor.h>

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
    actual_ret = forward_to_sxxxf(VariadicAdaptor::FS_printf, (void *) sprintf, actual_buffer, fmt,
                                  arg1, arg2, arg3);

    BOOST_VERIFY(actual_ret == expect_ret);
    BOOST_VERIFY(std::string_view(actual_buffer) == std::string_view(expect_buffer));

    // vsprintf
    actual_ret = forward_to_vsxxxf(VariadicAdaptor::FS_printf, (void *) vsprintf, actual_buffer,
                                   fmt, arg1, arg2, arg3);
    BOOST_VERIFY(actual_ret == expect_ret);
    BOOST_VERIFY(std::string_view(actual_buffer) == std::string_view(expect_buffer));
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
    actual_ret = forward_to_sxxxf(VariadicAdaptor::FS_scanf, (void *) sscanf, (char *) input, fmt,
                                  &actual_arg1, &actual_arg2, actual_arg3);
    BOOST_VERIFY(actual_ret == expect_ret);
    BOOST_VERIFY(actual_arg1 == expect_arg1);
    BOOST_VERIFY(actual_arg2 == expect_arg2);
    BOOST_VERIFY(std::string_view(actual_arg3) == std::string_view(expect_arg3));

    // vscanf
    actual_ret = forward_to_vsxxxf(VariadicAdaptor::FS_scanf, (void *) vsscanf, (char *) input, fmt,
                                   &actual_arg1, &actual_arg2, actual_arg3);
    BOOST_VERIFY(actual_ret == expect_ret);
    BOOST_VERIFY(actual_arg1 == expect_arg1);
    BOOST_VERIFY(actual_arg2 == expect_arg2);
    BOOST_VERIFY(std::string_view(actual_arg3) == std::string_view(expect_arg3));
}

BOOST_AUTO_TEST_SUITE_END()

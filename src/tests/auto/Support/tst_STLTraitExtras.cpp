// SPDX-License-Identifier: MIT

#include <tuple>
#include <type_traits>

#include <lorelei/Support/STLTraitExtras.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

namespace {

    // function_info on a plain function type: return type and argument tuple are recovered.
    using PlainFn = int(double, char);
    static_assert(std::is_same_v<function_info<PlainFn>::return_type, int>);
    static_assert(std::is_same_v<function_info<PlainFn>::tuple_type, std::tuple<double, char>>);

    // The same through a function pointer type.
    using PlainFnPtr = int (*)(double, char);
    static_assert(std::is_same_v<function_info<PlainFnPtr>::return_type, int>);
    static_assert(std::is_same_v<function_info<PlainFnPtr>::tuple_type, std::tuple<double, char>>);

    // A zero-argument function yields an empty tuple.
    static_assert(std::is_same_v<function_info<void()>::tuple_type, std::tuple<>>);

    // A trailing C variadic `...` is dropped from tuple_type, both as a type and as a pointer.
    using VariadicFn = int(double, char, ...);
    static_assert(std::is_same_v<function_info<VariadicFn>::return_type, int>);
    static_assert(std::is_same_v<function_info<VariadicFn>::tuple_type, std::tuple<double, char>>);

    using VariadicFnPtr = int (*)(double, char, ...);
    static_assert(std::is_same_v<function_info<VariadicFnPtr>::tuple_type, std::tuple<double, char>>);

    // An all-varargs function (only `...`) leaves an empty tuple.
    static_assert(std::is_same_v<function_info<void(...)>::tuple_type, std::tuple<>>);

    // return_type_of is the convenience alias over function_info.
    static_assert(std::is_same_v<return_type_of<PlainFn>, int>);
    static_assert(std::is_same_v<return_type_of<void(int)>, void>);

    // remove_attr_t yields the plain decayed type of a function value. Passing a function reference
    // collapses to the corresponding function pointer type.
    [[maybe_unused]] int sample(int) {
        return 0;
    }
    static_assert(std::is_same_v<remove_attr_t<&sample>, int (*)(int)>);
    static_assert(std::is_same_v<remove_attr_t<sample>, int (*)(int)>);

}

BOOST_AUTO_TEST_SUITE(test_STLTraitExtras)

// All of the checks above are static_asserts evaluated at compile time, so this case exists only so
// the test binary registers and runs. Reaching it means every trait expectation held.
BOOST_AUTO_TEST_CASE(compile_time_traits_hold) {
    BOOST_TEST(true);
}

BOOST_AUTO_TEST_SUITE_END()

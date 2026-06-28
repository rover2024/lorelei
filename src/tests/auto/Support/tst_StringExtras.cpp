// SPDX-License-Identifier: MIT

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <lorelei/Support/StringExtras.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

BOOST_AUTO_TEST_SUITE(test_StringExtras)

BOOST_AUTO_TEST_CASE(split_keeps_empty_tokens) {
    auto t = str::split(std::string_view("a,b,c"), ",");
    BOOST_TEST(t.size() == 3u);
    BOOST_TEST(t[0] == "a");
    BOOST_TEST(t[1] == "b");
    BOOST_TEST(t[2] == "c");

    // Empty tokens between / around delimiters are preserved.
    auto e = str::split(std::string_view("a,,c"), ",");
    BOOST_TEST(e.size() == 3u);
    BOOST_TEST(e[1] == "");

    // No delimiter -> the whole string as one token.
    auto one = str::split(std::string_view("abc"), ",");
    BOOST_TEST(one.size() == 1u);
    BOOST_TEST(one[0] == "abc");
}

BOOST_AUTO_TEST_CASE(join_is_split_inverse) {
    std::vector<std::string> v = {"a", "b", "c"};
    BOOST_TEST(str::join(v, ",") == "a,b,c");

    std::vector<std::string> single = {"only"};
    BOOST_TEST(str::join(single, ",") == "only");

    std::vector<std::string> empty;
    BOOST_TEST(str::join(empty, ",") == "");
}

BOOST_AUTO_TEST_CASE(formatN_substitutes_indexed_placeholders) {
    BOOST_TEST(str::formatN("%1-%2", "x", 3) == "x-3");
    BOOST_TEST(str::formatN("%1", 42) == "42");
    // "%%" is an escaped literal percent (only on the argument-substituting path).
    BOOST_TEST(str::formatN("%1%%", "x") == "x%");
}

BOOST_AUTO_TEST_CASE(formatN_handles_multiple_placeholders) {
    // 20 substitutions plus their separators overflow format()'s internal VarSizeArray<Part, 10>,
    // so this also exercises the container's heap growth. (Indices are multi-digit: %10..%20.)
    auto out = str::formatN(
        "%1.%2.%3.%4.%5.%6.%7.%8.%9.%10.%11.%12.%13.%14.%15.%16.%17.%18.%19.%20",
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
    BOOST_TEST(out == "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20");
}

BOOST_AUTO_TEST_CASE(varexp_expands_braced_variables) {
    std::map<std::string, std::string> vars = {{"a", "1"}, {"b", "2"}};
    BOOST_TEST(str::varexp("${a}/${b}", vars) == "1/2");
    // Unknown variables expand to the empty string.
    BOOST_TEST(str::varexp("x${z}y", vars) == "xy");
}

BOOST_AUTO_TEST_CASE(varexp_expands_nested_variables) {
    std::map<std::string, std::string> vars = {
        {"k", "a"}, {"a", "1"}, {"i", "1"}, {"x_1", "foo"}};
    // A nested ${...} is expanded first and its result is used as the variable name.
    BOOST_TEST(str::varexp("${${k}}", vars) == "1");     // ${k} -> a, then ${a} -> 1
    BOOST_TEST(str::varexp("${x_${i}}", vars) == "foo"); // x_${i} -> x_1, then -> foo
}

BOOST_AUTO_TEST_CASE(varexp_handles_multiple_variables) {
    // Likewise grows varexp()'s internal VarSizeArray<varexp_part, 10> past its inline buffer.
    std::map<std::string, std::string> vars;
    std::string input;
    std::string expected;
    for (int i = 1; i <= 20; ++i) {
        vars["v" + std::to_string(i)] = std::to_string(i * 10);
        input += "${v" + std::to_string(i) + "}";
        expected += std::to_string(i * 10);
        if (i != 20) {
            input += ",";
            expected += ",";
        }
    }
    BOOST_TEST(str::varexp(input, vars) == expected); // "10,20,...,200"
}

BOOST_AUTO_TEST_CASE(prefix_suffix_and_case) {
    BOOST_TEST(str::starts_with("foobar", "foo"));
    BOOST_TEST(!str::starts_with("foobar", "bar"));
    BOOST_TEST(str::ends_with("foobar", "bar"));
    BOOST_TEST(!str::ends_with("foobar", "foo"));

    BOOST_TEST(str::to_upper(std::string("aB1")) == "AB1");
    BOOST_TEST(str::to_lower(std::string("Ab1")) == "ab1");
}

BOOST_AUTO_TEST_SUITE_END()

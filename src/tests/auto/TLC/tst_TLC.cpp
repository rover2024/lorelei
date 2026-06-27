// SPDX-License-Identifier: MIT

#include <fstream>
#include <sstream>
#include <string>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// CMake generates the thunk for the ThunkExample fixture and compiles it (so a malformed emission
// fails the build); this test inspects the generated host source. The paths are handed to us as
// compile definitions (see CMakeLists.txt).

static std::string readFile(const char *path) {
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static const std::string &hostSrc() {
    static const std::string src = readFile(LORE_TLC_HOST_SRC);
    return src;
}

// Returns the GuestToHost Caller definition body of function \a name, or "" if absent. The trailing
// "::" targets the out-of-line definition (ProcFn<...>::invoke) rather than the forward declaration
// (struct ProcFn<...> {).
static std::string callerBody(const std::string &src, const std::string &name) {
    auto key = "ProcFn<::" + name + ", GuestToHost, Caller>::";
    auto pos = src.find(key);
    if (pos == std::string::npos) {
        return {};
    }
    auto end = src.find("\n}", pos);
    return src.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

// Whether function \a name is marshalled through the variadic adaptor (i.e. the format builder
// picked it up).
static bool isVariadic(const std::string &src, const std::string &name) {
    return callerBody(src, name).find("VariadicAdaptor") != std::string::npos;
}

BOOST_AUTO_TEST_SUITE(test_TLC)

BOOST_AUTO_TEST_CASE(stat_collects_requested_symbols) {
    auto content = readFile(LORE_TLC_STAT);
    BOOST_VERIFY(content.find("le_printf") != std::string::npos);
    BOOST_VERIFY(content.find("le_qsort") != std::string::npos);
    BOOST_VERIFY(content.find("le_mix") != std::string::npos);
    BOOST_VERIFY(content.find("le_compare_fn") != std::string::npos);
}

// le_mix is an ordinary (non-variadic) function, so it is not marshalled through the adaptor.
BOOST_AUTO_TEST_CASE(plain_function_is_not_variadic) {
    BOOST_VERIFY(!callerBody(hostSrc(), "le_mix").empty());
    BOOST_VERIFY(!isVariadic(hostSrc(), "le_mix"));
}

// A printf/scanf function is recognised by its name ending in "printf" / "scanf".
BOOST_AUTO_TEST_CASE(format_detected_by_name) {
    BOOST_VERIFY(isVariadic(hostSrc(), "le_printf"));
    BOOST_VERIFY(isVariadic(hostSrc(), "le_sscanf"));
}

// Regression: a printf/vprintf function tagged only by a ProcFnDesc must be marshalled. This guards
// the fix where the pass ID of an implicitly instantiated pass tag (pass::printf<>::ID) was read
// from the template pattern rather than the empty instantiation.
BOOST_AUTO_TEST_CASE(format_detected_by_descriptor) {
    BOOST_VERIFY(isVariadic(hostSrc(), "le_emit"));  // pass::printf  (...)
    BOOST_VERIFY(isVariadic(hostSrc(), "le_vemit")); // pass::vprintf (va_list)
}

// Regression: a printf function recognised only by its format attribute must be marshalled. This
// guards the fix where the `...` case checked firstToCheck == 0 instead of > 0 (and the va_list
// case lands in the vprintf pass).
BOOST_AUTO_TEST_CASE(format_detected_by_attribute) {
    BOOST_VERIFY(isVariadic(hostSrc(), "le_emit_attr"));  // format(printf, 2, 3), `...`
    BOOST_VERIFY(isVariadic(hostSrc(), "le_vemit_attr")); // format(printf, 2, 0), va_list
}

BOOST_AUTO_TEST_CASE(callback_is_substituted) {
    // The comparator is wrapped in a trampoline via the callback context.
    BOOST_VERIFY(hostSrc().find("CallbackContext_init") != std::string::npos);
}

// le_mix takes a long double, so the type filter is injected for it.
BOOST_AUTO_TEST_CASE(type_filter_is_applied) {
    BOOST_VERIFY(hostSrc().find("ProcArgFilter<long double>::filter") != std::string::npos);
    BOOST_VERIFY(hostSrc().find("ProcReturnFilter<long double>::filter") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

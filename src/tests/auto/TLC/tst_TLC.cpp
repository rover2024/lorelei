// SPDX-License-Identifier: MIT

#include <fstream>
#include <sstream>
#include <string>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// CMake generates the thunk for the ThunkExample fixture and compiles it (so a malformed emission
// fails the build). This test inspects the generated host and guest sources. The paths are handed
// to us as compile definitions (see CMakeLists.txt).

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

static const std::string &guestSrc() {
    static const std::string src = readFile(LORE_TLC_GUEST_SRC);
    return src;
}

// Returns the GuestToHost \a phase definition body of function \a name, or "" if absent. The
// trailing "::" targets the out-of-line definition (ProcFn<...>::invoke) rather than the forward
// declaration (struct ProcFn<...> {).
static std::string phaseBody(const std::string &src, const std::string &name, const char *phase) {
    auto key = "ProcFn<::" + name + ", GuestToHost, " + phase + ">::";
    auto pos = src.find(key);
    if (pos == std::string::npos) {
        return {};
    }
    auto end = src.find("\n}", pos);
    return src.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

static std::string callerBody(const std::string &src, const std::string &name) {
    return phaseBody(src, name, "Caller");
}

// Whether function \a name is marshalled through the variadic adaptor (i.e. the format builder
// picked it up). The host emits the adaptor call in the Caller, and the guest emits it in the Entry
// that packs the varargs before forwarding. Look in both.
static bool isVariadic(const std::string &src, const std::string &name) {
    return phaseBody(src, name, "Entry").find("VariadicAdaptor") != std::string::npos
        || phaseBody(src, name, "Caller").find("VariadicAdaptor") != std::string::npos;
}

BOOST_AUTO_TEST_SUITE(test_TLC)

BOOST_AUTO_TEST_CASE(stat_collects_requested_symbols) {
    auto content = readFile(LORE_TLC_STAT);
    BOOST_TEST(content.find("le_printf") != std::string::npos);
    BOOST_TEST(content.find("le_qsort") != std::string::npos);
    BOOST_TEST(content.find("le_mix") != std::string::npos);
    BOOST_TEST(content.find("le_compare_fn") != std::string::npos);
}

// le_mix is an ordinary (non-variadic) function, so it is not marshalled through the adaptor.
BOOST_AUTO_TEST_CASE(plain_function_is_not_variadic) {
    BOOST_TEST(!callerBody(hostSrc(), "le_mix").empty());
    BOOST_TEST(!isVariadic(hostSrc(), "le_mix"));
}

// A printf/scanf function is recognised by its name ending in "printf" / "scanf". Both sides
// marshal the varargs, so the adaptor must show up in the guest source as well as the host's.
BOOST_AUTO_TEST_CASE(format_detected_by_name) {
    BOOST_TEST(isVariadic(hostSrc(), "le_printf"));
    BOOST_TEST(isVariadic(hostSrc(), "le_sscanf"));
    BOOST_TEST(isVariadic(guestSrc(), "le_printf"));
    BOOST_TEST(isVariadic(guestSrc(), "le_sscanf"));
}

// Regression: a printf/vprintf function tagged only by a ProcFnDesc must be marshalled. This guards
// the fix where the pass ID of an implicitly instantiated pass tag (pass::printf<>::ID) was read
// from the template pattern rather than the empty instantiation.
BOOST_AUTO_TEST_CASE(format_detected_by_descriptor) {
    BOOST_TEST(isVariadic(hostSrc(), "le_emit"));   // pass::printf  (...)
    BOOST_TEST(isVariadic(hostSrc(), "le_vemit"));  // pass::vprintf (va_list)
    BOOST_TEST(isVariadic(guestSrc(), "le_emit"));
    BOOST_TEST(isVariadic(guestSrc(), "le_vemit"));
}

// Regression: a printf function recognised only by its format attribute must be marshalled. This
// guards the fix where the `...` case checked firstToCheck == 0 instead of > 0 (and the va_list
// case lands in the vprintf pass).
BOOST_AUTO_TEST_CASE(format_detected_by_attribute) {
    BOOST_TEST(isVariadic(hostSrc(), "le_emit_attr"));   // format(printf, 2, 3), `...`
    BOOST_TEST(isVariadic(hostSrc(), "le_vemit_attr"));  // format(printf, 2, 0), va_list
    BOOST_TEST(isVariadic(guestSrc(), "le_emit_attr"));
    BOOST_TEST(isVariadic(guestSrc(), "le_vemit_attr"));
}

BOOST_AUTO_TEST_CASE(callback_is_substituted) {
    // The comparator is wrapped in a trampoline via the callback context on the receiving (host)
    // side. The guest, which supplies the comparator, emits the matching ProcCb trampoline support.
    BOOST_TEST(hostSrc().find("CallbackContext") != std::string::npos);
    BOOST_TEST(guestSrc().find("ProcCb<le_compare_fn") != std::string::npos);
}

// le_mix takes a long double, so the type filter is injected for it. The filter is declared in the
// host manifest only, so it must appear in the host source and be absent from the guest source.
BOOST_AUTO_TEST_CASE(type_filter_is_applied) {
    BOOST_TEST(hostSrc().find("ProcArgFilter<long double>::filter") != std::string::npos);
    BOOST_TEST(hostSrc().find("ProcReturnFilter<long double>::filter") != std::string::npos);
    BOOST_TEST(guestSrc().find("ProcArgFilter<long double>") == std::string::npos);
    BOOST_TEST(guestSrc().find("ProcReturnFilter<long double>") == std::string::npos);
}

// The GTL's Entry is the real exported symbol the guest links against (aliased to ProcFn<...,Entry>
// ::invoke). The HTL emits none of these real-name exports, since the host side is reached through
// its registered dispatch entry rather than by symbol.
BOOST_AUTO_TEST_CASE(guest_exports_real_symbol_names) {
    BOOST_TEST(guestSrc().find("LORE_DECL_EXPORT int le_printf(") != std::string::npos);
    BOOST_TEST(hostSrc().find("LORE_DECL_EXPORT") == std::string::npos);
}

// The `...` printf/scanf functions are marshalled with VariadicAdaptor::call, while the va_list forms
// (the vprintf / vscanf passes) use ::vcall instead. This distinguishes the two builder families.
BOOST_AUTO_TEST_CASE(va_list_form_uses_vcall) {
    BOOST_TEST(callerBody(hostSrc(), "le_emit").find("VariadicAdaptor::call(") != std::string::npos);
    BOOST_TEST(callerBody(hostSrc(), "le_printf").find("VariadicAdaptor::call(") != std::string::npos);
    BOOST_TEST(callerBody(hostSrc(), "le_vemit").find("VariadicAdaptor::vcall(") != std::string::npos);
    BOOST_TEST(callerBody(hostSrc(), "le_vprintf").find("VariadicAdaptor::vcall(") != std::string::npos);
}

// Regression: a vprintf/vscanf Caller must still forward its fixed arguments (the format string and
// anything before it) in argv1. Dropping them left the host calling the target with the format slot
// taking the synthesised va_list and a null va_list, which crashed at run time. le_vprintf forwards
// its single fixed arg (fmt), and le_vemit forwards two (the channel and fmt).
BOOST_AUTO_TEST_CASE(va_list_form_forwards_fixed_args) {
    BOOST_TEST(callerBody(hostSrc(), "le_vprintf").find("CVargGet(arg1)") != std::string::npos);
    auto vemit = callerBody(hostSrc(), "le_vemit");
    BOOST_TEST(vemit.find("CVargGet(arg1)") != std::string::npos);
    BOOST_TEST(vemit.find("CVargGet(arg2)") != std::string::npos);
}

// A scanf-family function selects the ScanF kind, a printf-family one PrintF, and the format index
// follows the real parameter position: le_sscanf's format is its second argument, not the first.
BOOST_AUTO_TEST_CASE(format_kind_and_index) {
    BOOST_TEST(phaseBody(guestSrc(), "le_printf", "Entry").find("PrintF, arg1,") != std::string::npos);
    BOOST_TEST(phaseBody(guestSrc(), "le_sscanf", "Entry").find("ScanF, arg2,") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

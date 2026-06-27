// SPDX-License-Identifier: MIT

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/Tooling.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

namespace fs = std::filesystem;

// The fixture library (Desc.h, Symbols.conf, manifests) is copied next to the test binary; the
// LoreTLC executable and the lorelei include directories are handed to us as compile definitions
// (see CMakeLists.txt). These tests drive the real tool and inspect the source it emits.

static fs::path dataDir() {
    return fs::path(LORE_TEST_DATA_DIR);
}

static fs::path outDir() {
    auto d = fs::path(LORE_TEST_DATA_DIR).parent_path() / "tlc_out";
    fs::create_directories(d);
    return d;
}

static std::string includeArgs() {
    return std::string(" -I") + LORE_TLC_SOURCE_INCLUDE + " -I" + LORE_TLC_BUILD_INCLUDE;
}

static int runTlc(const std::string &args) {
    std::string cmd = std::string(LORE_TLC_EXECUTABLE) + " " + args;
    return std::system(cmd.c_str());
}

static std::string readFile(const fs::path &p) {
    std::ifstream f(p);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static fs::path runStat() {
    auto stat = outDir() / "ThunkStat.json";
    std::string args = "stat -o " + stat.string() + " -c " + (dataDir() / "Symbols.conf").string() +
                       " " + (dataDir() / "Desc.h").string() + " -- -xc++" + includeArgs();
    BOOST_VERIFY(runTlc(args) == 0);
    return stat;
}

// Runs stat then generate for the given mode, returning the path of the generated source.
static fs::path generateTo(const char *mode, const char *manifest) {
    auto stat = runStat();
    auto out = outDir() / (std::string("Thunk_") + mode + ".cpp");
    std::string args = "generate -o " + out.string() + " -s " + stat.string() + " -m " + mode +
                       " " + (dataDir() / manifest).string() + " -- -xc++" + includeArgs() + " -I" +
                       dataDir().string();
    BOOST_VERIFY(runTlc(args) == 0);
    return out;
}

// Reads the generated source for the given mode.
static std::string generate(const char *mode, const char *manifest) {
    return readFile(generateTo(mode, manifest));
}

// Syntax-checks a generated source in process with Clang, the same way the real thunk build would
// compile it. Returns true if it parses cleanly.
static bool compilesOk(const fs::path &src) {
    std::vector<std::string> args = {
        "-std=gnu++20", "-fsyntax-only",  "-fno-exceptions", "-fno-rtti",
        std::string("-I") + LORE_TLC_SOURCE_INCLUDE,
        std::string("-I") + LORE_TLC_BUILD_INCLUDE,
        std::string("-I") + dataDir().string(),
    };
    return clang::tooling::runToolOnCodeWithArgs(std::make_unique<clang::SyntaxOnlyAction>(),
                                                 readFile(src), args, src.string());
}

// Returns the GuestToHost Caller definition body of function \a name, or "" if absent. The
// trailing "::" targets the out-of-line definition (ProcFn<...>::invoke) rather than the forward
// declaration (struct ProcFn<...> {).
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

// The most basic guarantee: whatever TLC emits has to be valid C++.
BOOST_AUTO_TEST_CASE(generated_host_compiles) {
    BOOST_VERIFY(compilesOk(generateTo("host", "Manifest_host.cpp")));
}

BOOST_AUTO_TEST_CASE(generated_guest_compiles) {
    BOOST_VERIFY(compilesOk(generateTo("guest", "Manifest_guest.cpp")));
}

BOOST_AUTO_TEST_CASE(stat_collects_requested_symbols) {
    auto content = readFile(runStat());
    BOOST_VERIFY(content.find("le_printf") != std::string::npos);
    BOOST_VERIFY(content.find("le_qsort") != std::string::npos);
    BOOST_VERIFY(content.find("le_mix") != std::string::npos);
    BOOST_VERIFY(content.find("le_compare_fn") != std::string::npos);
}

// le_mix is an ordinary (non-variadic) function, so it is not marshalled through the adaptor.
BOOST_AUTO_TEST_CASE(plain_function_is_not_variadic) {
    auto src = generate("host", "Manifest_host.cpp");
    BOOST_VERIFY(!callerBody(src, "le_mix").empty());
    BOOST_VERIFY(!isVariadic(src, "le_mix"));
}

// A printf/scanf function is recognised by its name ending in "printf" / "scanf".
BOOST_AUTO_TEST_CASE(format_detected_by_name) {
    auto src = generate("host", "Manifest_host.cpp");
    BOOST_VERIFY(isVariadic(src, "le_printf"));
    BOOST_VERIFY(isVariadic(src, "le_sscanf"));
}

// Regression: a printf/vprintf function tagged only by a ProcFnDesc must be marshalled. This guards
// the fix where the pass ID of an implicitly instantiated pass tag (pass::printf<>::ID) was read
// from the template pattern rather than the empty instantiation.
BOOST_AUTO_TEST_CASE(format_detected_by_descriptor) {
    auto src = generate("host", "Manifest_host.cpp");
    BOOST_VERIFY(isVariadic(src, "le_emit"));  // pass::printf  (...)
    BOOST_VERIFY(isVariadic(src, "le_vemit")); // pass::vprintf (va_list)
}

// Regression: a printf function recognised only by its format attribute must be marshalled. This
// guards the fix where the `...` case checked firstToCheck == 0 instead of > 0 (and the va_list
// case lands in the vprintf pass).
BOOST_AUTO_TEST_CASE(format_detected_by_attribute) {
    auto src = generate("host", "Manifest_host.cpp");
    BOOST_VERIFY(isVariadic(src, "le_emit_attr"));  // format(printf, 2, 3), `...`
    BOOST_VERIFY(isVariadic(src, "le_vemit_attr")); // format(printf, 2, 0), va_list
}

BOOST_AUTO_TEST_CASE(callback_is_substituted) {
    auto src = generate("host", "Manifest_host.cpp");
    // The comparator is wrapped in a trampoline via the callback context.
    BOOST_VERIFY(src.find("CallbackContext_init") != std::string::npos);
}

// le_mix takes a long double, so the type filter is injected for it.
BOOST_AUTO_TEST_CASE(type_filter_is_applied) {
    auto src = generate("host", "Manifest_host.cpp");
    BOOST_VERIFY(src.find("ProcArgFilter<long double>::filter") != std::string::npos);
    BOOST_VERIFY(src.find("ProcReturnFilter<long double>::filter") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

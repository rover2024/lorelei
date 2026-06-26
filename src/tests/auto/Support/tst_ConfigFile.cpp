// SPDX-License-Identifier: MIT

#include <filesystem>
#include <string>

#include <lorelei/Support/ConfigFile.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

// The INI fixtures are copied next to the test binary by qm_add_copy_command; the
// directory is handed to us as a compile definition (see Support/CMakeLists.txt).
static std::filesystem::path dataFile(const char *name) {
    return std::filesystem::path(LORE_TEST_DATA_DIR) / name;
}

BOOST_AUTO_TEST_SUITE(test_ConfigFile)

BOOST_AUTO_TEST_CASE(parses_global_and_sections) {
    ConfigFile cf;
    BOOST_VERIFY(cf.load(dataFile("basic.ini")).success);

    // Global section + [server] + [flags].
    BOOST_VERIFY(cf.count() == 3u);
    BOOST_VERIFY(cf.global().name() == ConfigFile::GlobalSectionName);
    BOOST_VERIFY(cf.contains("server"));
    BOOST_VERIFY(cf.contains("flags"));
    BOOST_VERIFY(!cf.contains("missing"));

    // Keys before the first header live in the global section; quotes are stripped.
    BOOST_VERIFY(cf.global().getString("appname").value() == "lorelei");

    // A bare key (no '=') is present with an empty value.
    auto flags = cf.get("flags");
    BOOST_VERIFY(flags.has_value());
    BOOST_VERIFY(flags->get().contains("verbose"));
    BOOST_VERIFY(flags->get().getString("verbose").value() == "");
}

BOOST_AUTO_TEST_CASE(typed_accessors_and_defaults) {
    ConfigFile cf;
    BOOST_VERIFY(cf.load(dataFile("basic.ini")).success);
    const auto &g = cf.global();

    BOOST_VERIFY(g.getInt("threads").value() == 4);
    BOOST_VERIFY(g.getDouble("ratio").value() == 1.5);
    BOOST_VERIFY(g.getBool("debug").value() == true);

    // A non-numeric value yields nullopt, so the default is returned.
    BOOST_VERIFY(!g.getInt("appname").has_value());
    BOOST_VERIFY(g.getInt("appname", -1) == -1);

    // An empty value is not a valid number either.
    BOOST_VERIFY(g.getString("empty").value() == "");
    BOOST_VERIFY(!g.getInt("empty").has_value());

    // A missing key falls back to the supplied default.
    BOOST_VERIFY(g.getInt("nope", 7) == 7);
    BOOST_VERIFY(g.getBool("nope", true) == true);
}

BOOST_AUTO_TEST_CASE(duplicate_key_last_value_wins) {
    ConfigFile cf;
    BOOST_VERIFY(cf.load(dataFile("basic.ini")).success);
    auto server = cf.get("server");
    BOOST_VERIFY(server.has_value());

    // 'port' is assigned twice; the later assignment overwrites, in place.
    BOOST_VERIFY(server->get().getInt("port").value() == 9090);
}

BOOST_AUTO_TEST_CASE(quotes_and_inline_comments) {
    ConfigFile cf;
    BOOST_VERIFY(cf.load(dataFile("basic.ini")).success);
    const auto &server = cf.get("server")->get();

    // A '#' inside quotes is part of the value, not a comment marker.
    BOOST_VERIFY(server.getString("note").value() == "a # b");
    // The comment after an unquoted value is stripped and the value trimmed.
    BOOST_VERIFY(server.getString("trailing").value() == "keep me");
}

BOOST_AUTO_TEST_CASE(include_directive_merges_file) {
    ConfigFile cf;
    BOOST_VERIFY(cf.load(dataFile("main.ini")).success);

    // Both the host file's section and the included file's section are present.
    BOOST_VERIFY(cf.contains("main"));
    BOOST_VERIFY(cf.contains("sub"));
    BOOST_VERIFY(cf.get("main")->get().getInt("x").value() == 1);
    BOOST_VERIFY(cf.get("sub")->get().getInt("y").value() == 2);
}

BOOST_AUTO_TEST_CASE(error_on_unclosed_quote_resets_config) {
    ConfigFile cf;
    auto r = cf.load(dataFile("unterminated.ini"));
    BOOST_VERIFY(!r.success);
    BOOST_VERIFY(!r.errorMessage.empty());
    BOOST_VERIFY(r.line == 1);

    // A failed parse clears everything back to just the (empty) global section.
    BOOST_VERIFY(cf.count() == 1u);
    BOOST_VERIFY(cf.global().empty());
}

BOOST_AUTO_TEST_CASE(error_on_missing_file) {
    ConfigFile cf;
    auto r = cf.load(dataFile("does_not_exist.ini"));
    BOOST_VERIFY(!r.success);
    BOOST_VERIFY(!r.errorMessage.empty());
}

BOOST_AUTO_TEST_SUITE_END()

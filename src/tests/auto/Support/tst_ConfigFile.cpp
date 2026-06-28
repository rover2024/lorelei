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
    BOOST_TEST(cf.load(dataFile("basic.ini")).success);

    // Global section + [server] + [flags].
    BOOST_TEST(cf.count() == 3u);
    BOOST_TEST(cf.global().name() == ConfigFile::GlobalSectionName);
    BOOST_TEST(cf.contains("server"));
    BOOST_TEST(cf.contains("flags"));
    BOOST_TEST(!cf.contains("missing"));

    // Keys before the first header live in the global section; quotes are stripped.
    BOOST_TEST(cf.global().getString("appname").value() == "lorelei");

    // A bare key (no '=') is present with an empty value.
    auto flags = cf.get("flags");
    BOOST_TEST(flags.has_value());
    BOOST_TEST(flags->get().contains("verbose"));
    BOOST_TEST(flags->get().getString("verbose").value() == "");
}

BOOST_AUTO_TEST_CASE(typed_accessors_and_defaults) {
    ConfigFile cf;
    BOOST_TEST(cf.load(dataFile("basic.ini")).success);
    const auto &g = cf.global();

    BOOST_TEST(g.getInt("threads").value() == 4);
    BOOST_TEST(g.getDouble("ratio").value() == 1.5);
    BOOST_TEST(g.getBool("debug").value() == true);

    // A non-numeric value yields nullopt, so the default is returned.
    BOOST_TEST(!g.getInt("appname").has_value());
    BOOST_TEST(g.getInt("appname", -1) == -1);

    // An empty value is not a valid number either.
    BOOST_TEST(g.getString("empty").value() == "");
    BOOST_TEST(!g.getInt("empty").has_value());

    // A missing key falls back to the supplied default.
    BOOST_TEST(g.getInt("nope", 7) == 7);
    BOOST_TEST(g.getBool("nope", true) == true);
}

BOOST_AUTO_TEST_CASE(duplicate_key_last_value_wins) {
    ConfigFile cf;
    BOOST_TEST(cf.load(dataFile("basic.ini")).success);
    auto server = cf.get("server");
    BOOST_TEST(server.has_value());

    // 'port' is assigned twice; the later assignment overwrites, in place.
    BOOST_TEST(server->get().getInt("port").value() == 9090);
}

BOOST_AUTO_TEST_CASE(quotes_and_inline_comments) {
    ConfigFile cf;
    BOOST_TEST(cf.load(dataFile("basic.ini")).success);
    const auto &server = cf.get("server")->get();

    // A '#' inside quotes is part of the value, not a comment marker.
    BOOST_TEST(server.getString("note").value() == "a # b");
    // The comment after an unquoted value is stripped and the value trimmed.
    BOOST_TEST(server.getString("trailing").value() == "keep me");
}

BOOST_AUTO_TEST_CASE(include_directive_merges_file) {
    ConfigFile cf;
    BOOST_TEST(cf.load(dataFile("main.ini")).success);

    // Both the host file's section and the included file's section are present.
    BOOST_TEST(cf.contains("main"));
    BOOST_TEST(cf.contains("sub"));
    BOOST_TEST(cf.get("main")->get().getInt("x").value() == 1);
    BOOST_TEST(cf.get("sub")->get().getInt("y").value() == 2);
}

BOOST_AUTO_TEST_CASE(error_on_unclosed_quote_resets_config) {
    ConfigFile cf;
    auto r = cf.load(dataFile("unterminated.ini"));
    BOOST_TEST(!r.success);
    BOOST_TEST(!r.errorMessage.empty());
    BOOST_TEST(r.line == 1);

    // A failed parse clears everything back to just the (empty) global section.
    BOOST_TEST(cf.count() == 1u);
    BOOST_TEST(cf.global().empty());
}

BOOST_AUTO_TEST_CASE(error_on_missing_file) {
    ConfigFile cf;
    auto r = cf.load(dataFile("does_not_exist.ini"));
    BOOST_TEST(!r.success);
    BOOST_TEST(!r.errorMessage.empty());
}

BOOST_AUTO_TEST_SUITE_END()

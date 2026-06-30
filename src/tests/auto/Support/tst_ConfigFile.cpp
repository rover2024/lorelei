// SPDX-License-Identifier: MIT

#include <filesystem>
#include <fstream>
#include <string>

#include <lorelei/Support/ConfigFile.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

namespace {

    // Writes INI text to a uniquely named temporary file and removes it on destruction, so each
    // case can feed ConfigFile::load() a real path without any CMake fixture plumbing.
    struct TempIni {
        std::filesystem::path path;

        explicit TempIni(const std::string &content) {
            static int counter = 0;
            path = std::filesystem::temp_directory_path() /
                   ("lore_cfg_" + std::to_string(counter++) + ".ini");
            std::ofstream(path) << content;
        }
        ~TempIni() {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    };

    // The shared fixture exercised by most cases: a global section plus [server] and [flags],
    // covering quotes, inline comments, duplicate keys, bare keys and typed values.
    const char *kBasicIni = R"INI(# Global settings (keys before any section header land in the global section).
appname = "lorelei"
debug   = on
threads = 4
ratio   = 1.5
empty   =

[server]
host = localhost
port = 8080
port = 9090            # duplicate key: the last value wins
note = "a # b"         # a '#' inside quotes is not an inline comment
trailing = keep me     # the inline comment after the value is stripped

[flags]
verbose                # a bare key with no value
)INI";

}

BOOST_AUTO_TEST_SUITE(test_ConfigFile)

BOOST_AUTO_TEST_CASE(parses_global_and_sections) {
    TempIni ini(kBasicIni);
    ConfigFile cf;
    BOOST_TEST(cf.load(ini.path).success);

    // Global section + [server] + [flags].
    BOOST_TEST(cf.count() == 3u);
    BOOST_TEST(cf.global().name() == ConfigFile::GlobalSectionName);
    BOOST_TEST(cf.contains("server"));
    BOOST_TEST(cf.contains("flags"));
    BOOST_TEST(!cf.contains("missing"));

    // Keys before the first header live in the global section, and quotes are stripped.
    BOOST_TEST(cf.global().getString("appname").value() == "lorelei");

    // A bare key (no '=') is present with an empty value.
    auto flags = cf.get("flags");
    BOOST_TEST(flags.has_value());
    BOOST_TEST(flags->get().contains("verbose"));
    BOOST_TEST(flags->get().getString("verbose").value() == "");
}

BOOST_AUTO_TEST_CASE(typed_accessors_and_defaults) {
    TempIni ini(kBasicIni);
    ConfigFile cf;
    BOOST_TEST(cf.load(ini.path).success);
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
    TempIni ini(kBasicIni);
    ConfigFile cf;
    BOOST_TEST(cf.load(ini.path).success);
    auto server = cf.get("server");
    BOOST_TEST(server.has_value());

    // 'port' is assigned twice, and the later assignment overwrites, in place.
    BOOST_TEST(server->get().getInt("port").value() == 9090);
}

BOOST_AUTO_TEST_CASE(quotes_and_inline_comments) {
    TempIni ini(kBasicIni);
    ConfigFile cf;
    BOOST_TEST(cf.load(ini.path).success);
    const auto &server = cf.get("server")->get();

    // A '#' inside quotes is part of the value, not a comment marker.
    BOOST_TEST(server.getString("note").value() == "a # b");
    // The comment after an unquoted value is stripped and the value trimmed.
    BOOST_TEST(server.getString("trailing").value() == "keep me");
}

BOOST_AUTO_TEST_CASE(include_directive_merges_file) {
    // The host file pulls in another by an absolute path, which the include resolver honors as-is.
    TempIni sub("[sub]\ny = 2\n");
    TempIni main("[main]\nx = 1\n\ninclude " + sub.path.string() + "\n");

    ConfigFile cf;
    BOOST_TEST(cf.load(main.path).success);

    // Both the host file's section and the included file's section are present.
    BOOST_TEST(cf.contains("main"));
    BOOST_TEST(cf.contains("sub"));
    BOOST_TEST(cf.get("main")->get().getInt("x").value() == 1);
    BOOST_TEST(cf.get("sub")->get().getInt("y").value() == 2);
}

BOOST_AUTO_TEST_CASE(error_on_unclosed_quote_resets_config) {
    TempIni ini("key = \"unterminated\n");
    ConfigFile cf;
    auto r = cf.load(ini.path);
    BOOST_TEST(!r.success);
    BOOST_TEST(!r.errorMessage.empty());
    BOOST_TEST(r.line == 1);

    // A failed parse clears everything back to just the (empty) global section.
    BOOST_TEST(cf.count() == 1u);
    BOOST_TEST(cf.global().empty());
}

BOOST_AUTO_TEST_CASE(error_on_missing_file) {
    ConfigFile cf;
    auto r = cf.load(std::filesystem::temp_directory_path() / "lore_cfg_does_not_exist.ini");
    BOOST_TEST(!r.success);
    BOOST_TEST(!r.errorMessage.empty());
}

BOOST_AUTO_TEST_SUITE_END()

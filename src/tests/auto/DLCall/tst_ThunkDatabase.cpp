// SPDX-License-Identifier: MIT

#include <filesystem>
#include <fstream>
#include <map>
#include <string>

#include <lorelei/DLCall/ThunkDatabase.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

namespace {

    // Writes JSON to a uniquely named temporary file and removes it on destruction, so each case
    // can feed ThunkDatabase::load() a real path without any CMake fixture plumbing.
    struct TempJson {
        std::filesystem::path path;

        explicit TempJson(const std::string &content) {
            static int counter = 0;
            path = std::filesystem::temp_directory_path() /
                   ("lore_tdb_" + std::to_string(counter++) + ".json");
            std::ofstream(path) << content;
        }
        ~TempJson() {
            std::error_code ec;
            std::filesystem::remove(path, ec);
        }
    };

    // Convenience: compare a NUL-terminated entry field against an expected string.
    std::string s(const char *p) {
        return p ? std::string(p) : std::string("<null>");
    }

}

BOOST_AUTO_TEST_SUITE(test_ThunkDatabase)

BOOST_AUTO_TEST_CASE(load_failures) {
    ThunkDatabase db;

    // Malformed JSON.
    TempJson bad("{ this is not json");
    BOOST_TEST(!db.load(bad.path));

    // Valid JSON whose root is not an object.
    TempJson arr("[1, 2, 3]");
    BOOST_TEST(!db.load(arr.path));

    // Neither parse failure should have produced entries.
    BOOST_TEST(db.forwardThunks().empty());
    BOOST_TEST(db.reversedThunks().empty());
}

BOOST_AUTO_TEST_CASE(unreadable_path_is_a_load_failure) {
    // load() reports whether it could load what it was asked to: a missing or empty path is a failure
    // (the caller decides if that is acceptable). Either way the database is left empty.
    ThunkDatabase db;
    BOOST_TEST(!db.load(std::filesystem::temp_directory_path() / "lore_tdb_nope_xyz.json"));
    BOOST_TEST(!db.load({}));
    BOOST_TEST(db.forwardThunks().empty());
    BOOST_TEST(db.reversedThunks().empty());
}

BOOST_AUTO_TEST_CASE(forward_explicit_fields_and_lookup) {
    TempJson j(R"({
        "forwardThunks": [
            {
                "name": "zlib",
                "alias": ["z", "libz"],
                "guestThunk": "/g/zlib.so",
                "hostThunk": "/h/zlib_HTL.so",
                "hostLibrary": "libz.so.1"
            }
        ]
    })");

    ThunkDatabase db;
    BOOST_TEST(db.load(j.path));
    BOOST_TEST(db.forwardThunks().size() == 1u);

    const auto *fwd = db.forwardThunk("zlib");
    BOOST_REQUIRE(fwd != nullptr);
    BOOST_TEST(s(fwd->name) == "zlib");
    BOOST_TEST(s(fwd->guestThunk) == "/g/zlib.so");
    BOOST_TEST(s(fwd->hostThunk) == "/h/zlib_HTL.so");
    BOOST_TEST(s(fwd->hostLibrary) == "libz.so.1");
    BOOST_REQUIRE(fwd->aliasCount == 2u);
    BOOST_TEST(s(fwd->alias[0]) == "z");
    BOOST_TEST(s(fwd->alias[1]) == "libz");

    // Lookup by either alias returns the same entry.
    BOOST_TEST(db.forwardThunk("z") == fwd);
    BOOST_TEST(db.forwardThunk("libz") == fwd);

    // An unknown name returns null.
    BOOST_TEST(db.forwardThunk("missing") == nullptr);
}

BOOST_AUTO_TEST_CASE(forward_variable_expansion) {
    TempJson j(R"({
        "forwardThunks": [
            {
                "name": "foo",
                "guestThunk": "${GTL_DIR}/foo.so",
                "hostThunk": "${HTL_DIR}/foo_HTL.so",
                "hostLibrary": "${LIBNAME}"
            }
        ]
    })");

    ThunkDatabase db;
    const std::map<std::string, std::string> vars = {
        {"GTL_DIR", "/opt/gtl"},
        {"HTL_DIR", "/opt/htl"},
        {"LIBNAME", "libfoo.so.0"},
    };
    BOOST_TEST(db.load(j.path, vars));

    const auto *fwd = db.forwardThunk("foo");
    BOOST_REQUIRE(fwd != nullptr);
    BOOST_TEST(s(fwd->guestThunk) == "/opt/gtl/foo.so");
    BOOST_TEST(s(fwd->hostThunk) == "/opt/htl/foo_HTL.so");
    BOOST_TEST(s(fwd->hostLibrary) == "libfoo.so.0");
}

BOOST_AUTO_TEST_CASE(forward_defaults_from_dir_vars) {
    // With GTL_DIR / HTL_DIR set, an object may omit guestThunk / hostThunk / hostLibrary and have
    // them filled from the defaults: <GTL_DIR>/<name>.so, <HTL_DIR>/<name>_HTL.so, <name>.so.
    TempJson j(R"({
        "forwardThunks": [ { "name": "bar" } ]
    })");

    ThunkDatabase db;
    const std::map<std::string, std::string> vars = {
        {"GTL_DIR", "/d/gtl"},
        {"HTL_DIR", "/d/htl"},
    };
    BOOST_TEST(db.load(j.path, vars));

    const auto *fwd = db.forwardThunk("bar");
    BOOST_REQUIRE(fwd != nullptr);
    BOOST_TEST(s(fwd->guestThunk) == "/d/gtl/bar.so");
    BOOST_TEST(s(fwd->hostThunk) == "/d/htl/bar_HTL.so");
    BOOST_TEST(s(fwd->hostLibrary) == "bar.so");
}

BOOST_AUTO_TEST_CASE(forward_hostlibrary_defaults_to_name) {
    // hostLibrary is the one field that defaults even without any dir vars: <name>.so.
    TempJson j(R"({
        "forwardThunks": [
            { "name": "baz", "guestThunk": "/g/baz.so", "hostThunk": "/h/baz_HTL.so" }
        ]
    })");

    ThunkDatabase db;
    BOOST_TEST(db.load(j.path));

    const auto *fwd = db.forwardThunk("baz");
    BOOST_REQUIRE(fwd != nullptr);
    BOOST_TEST(s(fwd->hostLibrary) == "baz.so");
}

BOOST_AUTO_TEST_CASE(forward_shorthand_string_form) {
    // A bare string entry is only honored when both dir defaults are present.
    TempJson j(R"({ "forwardThunks": [ "quux" ] })");

    // Without defaults the shorthand entry is dropped.
    {
        ThunkDatabase db;
        BOOST_TEST(db.load(j.path));
        BOOST_TEST(db.forwardThunks().empty());
    }

    // With defaults it is expanded like the omitted-field object form.
    {
        ThunkDatabase db;
        const std::map<std::string, std::string> vars = {
            {"GTL_DIR", "/s/gtl"},
            {"HTL_DIR", "/s/htl"},
        };
        BOOST_TEST(db.load(j.path, vars));
        const auto *fwd = db.forwardThunk("quux");
        BOOST_REQUIRE(fwd != nullptr);
        BOOST_TEST(s(fwd->guestThunk) == "/s/gtl/quux.so");
        BOOST_TEST(s(fwd->hostThunk) == "/s/htl/quux_HTL.so");
        BOOST_TEST(s(fwd->hostLibrary) == "quux.so");
    }
}

BOOST_AUTO_TEST_CASE(forward_skips_invalid_entries) {
    // No name -> skipped. Missing guestThunk with no default -> skipped. Only the valid entry stays.
    TempJson j(R"({
        "forwardThunks": [
            { "guestThunk": "/g/x.so", "hostThunk": "/h/x_HTL.so" },
            { "name": "noguest", "hostThunk": "/h/y_HTL.so" },
            { "name": "ok", "guestThunk": "/g/ok.so", "hostThunk": "/h/ok_HTL.so" }
        ]
    })");

    ThunkDatabase db;
    BOOST_TEST(db.load(j.path));
    BOOST_TEST(db.forwardThunks().size() == 1u);
    BOOST_TEST(db.forwardThunk("ok") != nullptr);
    BOOST_TEST(db.forwardThunk("noguest") == nullptr);
}

BOOST_AUTO_TEST_CASE(reversed_basic_fields_and_lookup) {
    TempJson j(R"({
        "reversedThunks": [
            {
                "name": "libfoo",
                "alias": ["foo"],
                "fileName": "libfoo.so.1",
                "thunks": ["zlib", "png"]
            }
        ]
    })");

    ThunkDatabase db;
    BOOST_TEST(db.load(j.path));
    BOOST_TEST(db.reversedThunks().size() == 1u);

    const auto *rev = db.reversedThunk("libfoo");
    BOOST_REQUIRE(rev != nullptr);
    BOOST_TEST(s(rev->name) == "libfoo");
    BOOST_TEST(s(rev->fileName) == "libfoo.so.1");
    BOOST_REQUIRE(rev->aliasCount == 1u);
    BOOST_TEST(s(rev->alias[0]) == "foo");
    BOOST_REQUIRE(rev->thunksCount == 2u);
    BOOST_TEST(s(rev->thunks[0]) == "zlib");
    BOOST_TEST(s(rev->thunks[1]) == "png");

    BOOST_TEST(db.reversedThunk("foo") == rev);
    BOOST_TEST(db.reversedThunk("nope") == nullptr);
}

BOOST_AUTO_TEST_CASE(reversed_skips_invalid_entries) {
    // A reversed entry needs both name and fileName, otherwise it is dropped.
    TempJson j(R"({
        "reversedThunks": [
            { "fileName": "noname.so" },
            { "name": "nofile" },
            { "name": "good", "fileName": "good.so" }
        ]
    })");

    ThunkDatabase db;
    BOOST_TEST(db.load(j.path));
    BOOST_TEST(db.reversedThunks().size() == 1u);
    BOOST_TEST(db.reversedThunk("good") != nullptr);
    BOOST_TEST(db.reversedThunk("nofile") == nullptr);
}

BOOST_AUTO_TEST_CASE(reload_clears_previous_state) {
    TempJson first(R"({ "forwardThunks": [
        { "name": "alpha", "guestThunk": "/g/a.so", "hostThunk": "/h/a_HTL.so" } ] })");
    TempJson second(R"({ "forwardThunks": [
        { "name": "beta", "guestThunk": "/g/b.so", "hostThunk": "/h/b_HTL.so" } ] })");

    ThunkDatabase db;
    BOOST_TEST(db.load(first.path));
    BOOST_TEST(db.forwardThunk("alpha") != nullptr);

    BOOST_TEST(db.load(second.path));
    BOOST_TEST(db.forwardThunks().size() == 1u);
    BOOST_TEST(db.forwardThunk("beta") != nullptr);
    // The entry from the first load must be gone.
    BOOST_TEST(db.forwardThunk("alpha") == nullptr);
}

BOOST_AUTO_TEST_CASE(loadpack_shorthand_defaults_from_dirs) {
    // loadPack() supplies GTL_DIR / HTL_DIR from its dir arguments, so a pack JSON may use the shorthand
    // and omitted-field forms just like a top-level load with those vars.
    TempJson pack(R"({ "forwardThunks": [ "libz", { "name": "liblzma" } ] })");

    ThunkDatabase db;  // a fresh database is already empty; no top-level load needed
    BOOST_TEST(db.loadPack(pack.path, "/p/gtl", "/p/htl", {}));

    const auto *z = db.forwardThunk("libz");
    BOOST_REQUIRE(z != nullptr);
    BOOST_TEST(s(z->guestThunk) == "/p/gtl/libz.so");
    BOOST_TEST(s(z->hostThunk) == "/p/htl/libz_HTL.so");
    BOOST_TEST(s(z->hostLibrary) == "libz.so");
    BOOST_TEST(db.forwardThunk("liblzma") != nullptr);
}

BOOST_AUTO_TEST_CASE(loadpack_layers_under_override) {
    // A pack is the lowest priority: it fills in new names but never displaces an entry already present
    // (the override JSON, or an earlier pack).
    TempJson ovr(R"({ "forwardThunks": [
        { "name": "foo", "guestThunk": "/ovr/foo.so", "hostThunk": "/ovr/foo_HTL.so" } ] })");
    TempJson pack(R"({ "forwardThunks": [
        { "name": "foo", "guestThunk": "/pack/foo.so", "hostThunk": "/pack/foo_HTL.so" },
        { "name": "bar", "guestThunk": "/pack/bar.so", "hostThunk": "/pack/bar_HTL.so" } ] })");

    ThunkDatabase db;
    BOOST_TEST(db.load(ovr.path));
    BOOST_TEST(db.loadPack(pack.path, "/p/gtl", "/p/htl", {}));

    // foo stays the override's, and bar comes from the pack.
    const auto *foo = db.forwardThunk("foo");
    BOOST_REQUIRE(foo != nullptr);
    BOOST_TEST(s(foo->guestThunk) == "/ovr/foo.so");
    BOOST_TEST(db.forwardThunk("bar") != nullptr);
    BOOST_TEST(db.forwardThunks().size() == 2u);
}

BOOST_AUTO_TEST_CASE(materialize_forward_adds_and_is_idempotent) {
    ThunkDatabase db;  // fresh, empty

    const auto *e = db.materializeForward("qux", "/g/qux.so", "/h/qux_HTL.so", "qux.so");
    BOOST_REQUIRE(e != nullptr);
    BOOST_TEST(s(e->guestThunk) == "/g/qux.so");
    BOOST_TEST(s(e->hostThunk) == "/h/qux_HTL.so");
    BOOST_TEST(s(e->hostLibrary) == "qux.so");
    BOOST_TEST(db.forwardThunk("qux") == e);

    // Materializing the same name again keeps the existing entry (and its pointer) unchanged.
    const auto *again = db.materializeForward("qux", "/other/qux.so", "/other/qux_HTL.so", "qux.so");
    BOOST_TEST(again == e);
    BOOST_TEST(s(again->guestThunk) == "/g/qux.so");
    BOOST_TEST(db.forwardThunks().size() == 1u);
}

BOOST_AUTO_TEST_CASE(materialize_does_not_override_json) {
    // Convention never displaces an explicit JSON entry.
    TempJson j(R"({ "forwardThunks": [
        { "name": "foo", "guestThunk": "/json/foo.so", "hostThunk": "/json/foo_HTL.so" } ] })");

    ThunkDatabase db;
    BOOST_TEST(db.load(j.path));

    const auto *e = db.materializeForward("foo", "/conv/foo.so", "/conv/foo_HTL.so", "foo.so");
    BOOST_REQUIRE(e != nullptr);
    BOOST_TEST(s(e->guestThunk) == "/json/foo.so");
    BOOST_TEST(db.forwardThunks().size() == 1u);
}

BOOST_AUTO_TEST_SUITE_END()

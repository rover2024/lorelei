// SPDX-License-Identifier: MIT

#include <cstdio>
#include <string>

#include <lorelei/Support/Logging.h>

#ifdef _WIN32
#  include <io.h>
#  define lore_dup    _dup
#  define lore_dup2   _dup2
#  define lore_close  _close
#  define lore_fileno _fileno
#  define NullDevice  "NUL"
#else
#  include <unistd.h>
#  define lore_dup    dup
#  define lore_dup2   dup2
#  define lore_close  close
#  define lore_fileno fileno
#  define NullDevice  "/dev/null"
#endif

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using namespace lore;

// The filter state (rules + installed filter + callback) lives in a process-global registry, so
// every case sets up its own state and a guard restores the defaults on the way out, even if an
// assertion throws mid-case.
namespace {
    void setRules(const std::string &rules) {
        LogCategory::defaultCategory().setFilterRules(rules);
    }

    struct LoggingGuard {
        ~LoggingGuard() {
            LogCategory::setLogFilter(nullptr);                  // back to the default filter
            LogCategory::defaultCategory().setFilterRules("");   // ...and an empty (all-on) ruleset
        }
    };

    bool allEnabled(const LogCategory &c) {
        for (int level = Logger::Trace; level <= Logger::Fatal; ++level) {
            if (!c.isLevelEnabled(level))
                return false;
        }
        return true;
    }

    bool allDisabled(const LogCategory &c) {
        for (int level = Logger::Trace; level <= Logger::Fatal; ++level) {
            if (c.isLevelEnabled(level))
                return false;
        }
        return true;
    }

    // A capturing sink used by the end-to-end test.
    int g_emitCount = 0;
    int g_lastLevel = 0;
    void captureSink(int level, const LogContext &, const std::string_view &) {
        ++g_emitCount;
        g_lastLevel = level;
    }

    // Redirects stdout and stderr into a scratch file for as long as it lives, and hands back
    // what was written. The default sink writes to them directly, so this is the only way to see
    // what it produced.
    class CapturedOutput {
    public:
        CapturedOutput() {
            std::fflush(stdout);
            std::fflush(stderr);
            _sink = std::tmpfile();
            _out = lore_dup(lore_fileno(stdout));
            _err = lore_dup(lore_fileno(stderr));
            if (_sink) {
                lore_dup2(lore_fileno(_sink), lore_fileno(stdout));
                lore_dup2(lore_fileno(_sink), lore_fileno(stderr));
            }
        }

        ~CapturedOutput() {
            std::fflush(stdout);
            std::fflush(stderr);
            lore_dup2(_out, lore_fileno(stdout));
            lore_dup2(_err, lore_fileno(stderr));
            lore_close(_out);
            lore_close(_err);
            if (_sink) {
                std::fclose(_sink);
            }
        }

        std::string contents() const {
            std::fflush(stdout);
            std::fflush(stderr);
            if (!_sink) {
                return {};
            }
            std::fseek(_sink, 0, SEEK_SET);
            std::string text;
            char buf[1024];
            size_t n;
            while ((n = std::fread(buf, 1, sizeof(buf), _sink)) > 0) {
                text.append(buf, n);
            }
            return text;
        }

    private:
        FILE *_sink = nullptr;
        int _out = -1;
        int _err = -1;
    };
} // namespace

BOOST_AUTO_TEST_SUITE(test_Logging)

// --- Baseline ---------------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(all_levels_enabled_without_rules) {
    LoggingGuard guard;
    LogCategory c("lore.test.fresh");
    BOOST_TEST(allEnabled(c));
}

// --- Category matching modes ------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(exact_rule_disables_whole_category) {
    LoggingGuard guard;
    LogCategory hit("lore.exact");
    LogCategory miss("lore.exact.sub");

    setRules("lore.exact=false");

    // An exact pattern matches only the identical name, across every level.
    BOOST_TEST(allDisabled(hit));
    BOOST_TEST(allEnabled(miss));
}

BOOST_AUTO_TEST_CASE(prefix_wildcard_matches_subtree) {
    LoggingGuard guard;
    setRules("lore.io.*=false");

    LogCategory in("lore.io.socket"); // constructed after the rule -> picked up in ctor
    LogCategory out("lore.gfx");
    BOOST_TEST(allDisabled(in));
    BOOST_TEST(allEnabled(out));
}

BOOST_AUTO_TEST_CASE(suffix_wildcard_matches_tail) {
    LoggingGuard guard;
    setRules("*.io=false");

    LogCategory in("lore.io");
    LogCategory out("lore.ioext");
    BOOST_TEST(allDisabled(in));
    BOOST_TEST(allEnabled(out));
}

BOOST_AUTO_TEST_CASE(contains_wildcard_matches_substring) {
    LoggingGuard guard;
    setRules("*io*=false");

    LogCategory in("lore.iostream.x");
    LogCategory out("lore.gfx");
    BOOST_TEST(allDisabled(in));
    BOOST_TEST(allEnabled(out));
}

BOOST_AUTO_TEST_CASE(bare_star_matches_everything) {
    LoggingGuard guard;
    setRules("*=false");

    LogCategory a("anything");
    LogCategory b("lore.deep.name");
    BOOST_TEST(allDisabled(a));
    BOOST_TEST(allDisabled(b));
}

// --- Level selectors --------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(level_token_targets_single_level) {
    LoggingGuard guard;
    LogCategory c("lore.level");
    setRules("lore.level.debug=false");

    BOOST_TEST(!c.isLevelEnabled(Logger::Debug));
    BOOST_TEST(c.isLevelEnabled(Logger::Warning));
    BOOST_TEST(c.isLevelEnabled(Logger::Trace));
}

BOOST_AUTO_TEST_CASE(every_level_token_is_recognized) {
    LoggingGuard guard;
    struct {
        const char *token;
        int level;
    } cases[] = {
        {"trace", Logger::Trace},        {"debug", Logger::Debug},
        {"success", Logger::Success},    {"info", Logger::Information},
        {"information", Logger::Information}, {"warning", Logger::Warning},
        {"critical", Logger::Critical},  {"fatal", Logger::Fatal},
    };
    for (const auto &tc : cases) {
        LogCategory c("lore.tok");
        setRules(std::string("lore.tok.") + tc.token + "=false");

        BOOST_TEST(!c.isLevelEnabled(tc.level));
        // Only the named level is affected, and a different one stays enabled.
        int other = (tc.level == Logger::Warning) ? Logger::Trace : Logger::Warning;
        BOOST_TEST(c.isLevelEnabled(other));

        setRules("");
    }
}

// --- Ordering & separators --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(later_rule_overrides_earlier) {
    LoggingGuard guard;
    LogCategory c("lore.order");
    // Disable everything, then re-enable just warnings.
    setRules("lore.order=false\nlore.order.warning=true");

    BOOST_TEST(c.isLevelEnabled(Logger::Warning));
    BOOST_TEST(!c.isLevelEnabled(Logger::Debug));
    BOOST_TEST(!c.isLevelEnabled(Logger::Critical));
}

BOOST_AUTO_TEST_CASE(semicolons_and_comments_are_handled) {
    LoggingGuard guard;
    LogCategory c("anything.at.all");
    // '#' lines are comments, and ';' separates rules just like a newline.
    setRules("# turn everything off\n*=false ; anything.at.all.info=true");

    BOOST_TEST(c.isLevelEnabled(Logger::Information));
    BOOST_TEST(!c.isLevelEnabled(Logger::Debug));
}

// --- Boolean spellings ------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(boolean_spellings_and_case) {
    LoggingGuard guard;
    LogCategory tTrue("lore.btrue");
    LogCategory tOne("lore.bone");
    LogCategory tUpper("lore.bupper");
    LogCategory fFalse("lore.bfalse");
    LogCategory fZero("lore.bzero");

    setRules("*=false\n"        // baseline: everything off, then re-enable selectively
             "lore.btrue=true\n"
             "lore.bone=1\n"
             "lore.bupper=TRUE\n"
             "lore.bfalse=false\n"
             "lore.bzero=0\n");

    BOOST_TEST(tTrue.isLevelEnabled(Logger::Warning));
    BOOST_TEST(tOne.isLevelEnabled(Logger::Warning));
    BOOST_TEST(tUpper.isLevelEnabled(Logger::Warning)); // case-insensitive
    BOOST_TEST(!fFalse.isLevelEnabled(Logger::Warning));
    BOOST_TEST(!fZero.isLevelEnabled(Logger::Warning));
}

// --- Malformed / ambiguous input --------------------------------------------------------------

BOOST_AUTO_TEST_CASE(malformed_rules_are_ignored) {
    LoggingGuard guard;
    LogCategory c("lore.bad");
    // No '=' (skipped), a mid-string '*' (unsupported pattern), and a non-boolean value (skipped).
    setRules("lore.bad\n"
             "lo*re.bad=false\n"
             "lore.bad=maybe");
    BOOST_TEST(allEnabled(c));
}

BOOST_AUTO_TEST_CASE(unknown_level_token_is_part_of_category_name) {
    LoggingGuard guard;
    LogCategory exact("lore.x.bogus");
    LogCategory parent("lore.x");
    setRules("lore.x.bogus=false");

    // "bogus" is not a level token, so the whole string is treated as an exact category name.
    BOOST_TEST(allDisabled(exact));
    BOOST_TEST(allEnabled(parent));
}

// --- Getter round-trip & clearing -------------------------------------------------------------

BOOST_AUTO_TEST_CASE(filter_rules_getter_roundtrips) {
    LoggingGuard guard;
    setRules("lore.x=false");
    BOOST_TEST(LogCategory::filterRules() == "lore.x=false");
}

BOOST_AUTO_TEST_CASE(clearing_rules_restores_baseline) {
    LoggingGuard guard;
    LogCategory c("lore.reset");
    setRules("lore.reset=false");
    BOOST_TEST(!c.isLevelEnabled(Logger::Warning));

    setRules("");
    BOOST_TEST(allEnabled(c));
}

// --- Custom filter interaction (mirrors Qt's installFilter overriding the rules) ---------------

BOOST_AUTO_TEST_CASE(custom_filter_replaces_rule_based_default) {
    LoggingGuard guard;

    // A custom filter disables everything, regardless of any rules.
    LogCategory::setLogFilter([](LogCategory *cat) {
        for (int level = Logger::Trace; level <= Logger::Fatal; ++level)
            cat->setLevelEnabled(level, false);
    });

    LogCategory c("lore.custom");
    BOOST_TEST(allDisabled(c)); // custom filter applied on registration

    // Rules no longer take effect, because only the default filter consults them.
    setRules("lore.custom=true");
    BOOST_TEST(allDisabled(c));

    // Restoring the default filter makes the rule effective again.
    LogCategory::setLogFilter(nullptr);
    BOOST_TEST(c.isLevelEnabled(Logger::Warning));
}

// --- End-to-end: rules actually gate emission -------------------------------------------------

BOOST_AUTO_TEST_CASE(disabled_level_does_not_reach_the_callback) {
    LoggingGuard guard;
    auto prev = Logger::logCallback();
    Logger::setLogCallback(captureSink);

    LogCategory c("lore.emit");
    setRules("lore.emit.debug=false");

    g_emitCount = 0;
    g_lastLevel = 0;
    c.log<Logger::Debug>(__FILE__, __LINE__, __FUNCTION__, "should be suppressed");
    int afterDebug = g_emitCount;
    c.log<Logger::Warning>(__FILE__, __LINE__, __FUNCTION__, "should be emitted");
    int afterWarning = g_emitCount;
    int lastLevel = g_lastLevel;

    Logger::setLogCallback(prev); // restore before asserting so a failure can't leak the sink

    BOOST_TEST(afterDebug == 0);   // the disabled level never invoked the callback
    BOOST_TEST(afterWarning == 1); // the enabled level did
    BOOST_TEST(lastLevel == Logger::Warning);
}

// --- The default sink ---------------------------------------------------------------------

// Every case above replaces the sink, so the built-in one had never been run. It dropped
// Information, which is 4 against Success's 3 and so clears the level gate and reaches the
// switch, where it had no case of its own. A debug build aborted on the assert there, a release
// build compiled the assert away and lost the message quietly, which is why surviving the call
// is not enough to check.
BOOST_AUTO_TEST_CASE(default_sink_emits_every_level_at_or_above_success) {
    LoggingGuard guard;
    Logger::setLogCallback(nullptr); // put the built-in one back

    std::string text;
    {
        CapturedOutput captured;
        LogContext context(__FILE__, __LINE__, __FUNCTION__, "lore.sink");
        Logger(context).print(Logger::Trace, "trace-line");
        Logger(context).print(Logger::Debug, "debug-line");
        Logger(context).print(Logger::Success, "success-line");
        Logger(context).print(Logger::Information, "information-line");
        Logger(context).print(Logger::Warning, "warning-line");
        Logger(context).print(Logger::Critical, "critical-line");
        Logger(context).print(Logger::Fatal, "fatal-line");
        text = captured.contents();
    }

    // below the gate, so deliberately absent
    BOOST_TEST(text.find("trace-line") == std::string::npos);
    BOOST_TEST(text.find("debug-line") == std::string::npos);

    // at or above it, so every one of them has to appear
    BOOST_TEST(text.find("success-line") != std::string::npos, text);
    BOOST_TEST(text.find("information-line") != std::string::npos, text);
    BOOST_TEST(text.find("warning-line") != std::string::npos, text);
    BOOST_TEST(text.find("critical-line") != std::string::npos, text);
    BOOST_TEST(text.find("fatal-line") != std::string::npos, text);
}

// print() takes an int rather than a Level, so a caller can invent one. That must neither abort
// nor swallow the message.
BOOST_AUTO_TEST_CASE(default_sink_takes_a_level_outside_the_enum) {
    LoggingGuard guard;
    Logger::setLogCallback(nullptr);

    std::string text;
    {
        CapturedOutput captured;
        LogContext context(__FILE__, __LINE__, __FUNCTION__, "lore.sink");
        Logger(context).print(42, "invented-level");
        Logger(context).print(-1, "negative-level"); // below the gate, dropped
        text = captured.contents();
    }

    BOOST_TEST(text.find("invented-level") != std::string::npos, text);
    BOOST_TEST(text.find("negative-level") == std::string::npos);
}

// nullptr means the built-in sink, the way it already does for setLogFilter(). Assigning it raw
// would leave a null pointer for the next record to call through.
BOOST_AUTO_TEST_CASE(null_callback_restores_the_default) {
    LoggingGuard guard;
    auto original = Logger::logCallback();

    Logger::setLogCallback(captureSink);
    BOOST_TEST(Logger::logCallback() == captureSink);

    Logger::setLogCallback(nullptr);
    BOOST_TEST(Logger::logCallback() != nullptr);
    BOOST_TEST(Logger::logCallback() == original);

    std::string text;
    {
        CapturedOutput captured;
        loreWarning("built-in-sink-reached");
        text = captured.contents();
    }
    BOOST_TEST(text.find("built-in-sink-reached") != std::string::npos, text);
}

BOOST_AUTO_TEST_SUITE_END()
// SPDX-License-Identifier: MIT

#include "Logging.h"

#include <cstdarg>
#include <cstdlib>
#include <optional>
#include <shared_mutex>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <cassert>

namespace lore {

    static void defaultLogCallback(int level, const LogContext &context,
                                   const std::string_view &message);

    static void defaultLogCategoryFilter(LogCategory *category);

    /// LoggingRule - A single parsed entry of the filter-rules string.
    ///
    /// Mirrors the `category[.level] = bool` grammar of Qt's QLoggingCategory rules. The category
    /// pattern may carry a single leading and/or trailing `*` wildcard, which is folded into a
    /// match mode; `level == 0` means the rule applies to every level.
    struct LoggingRule {
        enum MatchMode {
            Exact,    // "foo"   -> name == text
            Prefix,   // "foo*"  -> name starts with text
            Suffix,   // "*foo"  -> name ends with text
            Contains, // "*foo*" -> name contains text
        };

        std::string text;       // category pattern with wildcards stripped
        MatchMode mode = Exact;
        int level = 0;          // 0 == all levels, otherwise a Logger::Level
        bool enable = false;

        bool matchesCategory(const std::string_view &name) const {
            switch (mode) {
                case Prefix:
                    return str::starts_with(name, text);
                case Suffix:
                    return str::ends_with(name, text);
                case Contains:
                    return str::contains(name, text);
                default:
                    return name == text;
            }
        }

        bool appliesToLevel(int lv) const {
            return level == 0 || level == lv;
        }
    };

    // Map a rule's trailing `.<level>` token to a Logger::Level (0 if it is not a level token).
    static int levelFromToken(const std::string_view &token) {
        if (token == "trace")
            return Logger::Trace;
        if (token == "debug")
            return Logger::Debug;
        if (token == "success")
            return Logger::Success;
        if (token == "info" || token == "information")
            return Logger::Information;
        if (token == "warning")
            return Logger::Warning;
        if (token == "critical")
            return Logger::Critical;
        if (token == "fatal")
            return Logger::Fatal;
        return 0;
    }

    static std::optional<bool> parseBool(const std::string_view &value) {
        std::string v = str::to_lower(std::string(value));
        if (v == "true" || v == "1")
            return true;
        if (v == "false" || v == "0")
            return false;
        return std::nullopt;
    }

    // Parse a single `category[.level] = bool` rule. Returns nullopt for malformed/unsupported input.
    static std::optional<LoggingRule> parseRule(const std::string_view &line) {
        auto eq = line.find('=');
        if (eq == std::string_view::npos)
            return std::nullopt;

        std::string_view pattern = str::trim(line.substr(0, eq));
        auto value = parseBool(str::trim(line.substr(eq + 1)));
        if (pattern.empty() || !value)
            return std::nullopt;

        LoggingRule rule;
        rule.enable = *value;

        // A trailing `.<token>` is a level selector only if it names a known level.
        if (auto dot = pattern.rfind('.'); dot != std::string_view::npos) {
            if (int level = levelFromToken(pattern.substr(dot + 1))) {
                rule.level = level;
                pattern = pattern.substr(0, dot);
            }
        }

        // Fold a single leading/trailing '*' into the match mode.
        bool left = str::starts_with(pattern, '*');
        bool right = str::ends_with(pattern, '*');
        if (left)
            pattern = str::drop_front(pattern);
        if (right && !pattern.empty())
            pattern = str::drop_back(pattern);

        if (left && right)
            rule.mode = LoggingRule::Contains;
        else if (left)
            rule.mode = LoggingRule::Suffix;
        else if (right)
            rule.mode = LoggingRule::Prefix;
        else
            rule.mode = LoggingRule::Exact;

        // Wildcards are only supported at the ends; a leftover '*' is an unsupported pattern.
        if (str::contains(pattern, '*'))
            return std::nullopt;
        if (rule.mode == LoggingRule::Exact && pattern.empty())
            return std::nullopt;

        rule.text = std::string(pattern);
        return rule;
    }

    // Parse a full rules string: entries separated by newlines or ';', '#'-prefixed lines ignored.
    static std::vector<LoggingRule> parseRules(const std::string_view &rules) {
        std::vector<LoggingRule> result;
        for (const auto &rawLine : str::split(rules, "\n")) {
            for (const auto &rawEntry : str::split(rawLine, ";")) {
                std::string_view entry = str::trim(rawEntry);
                if (entry.empty() || str::starts_with(entry, '#'))
                    continue;
                if (auto rule = parseRule(entry))
                    result.push_back(std::move(*rule));
            }
        }
        return result;
    }

    class LogRegistry {
    public:
        static inline Logger::LogCallback callback = defaultLogCallback;
        static inline LogCategory::LogCategoryFilter categoryFilter = defaultLogCategoryFilter;

        std::string filterRules;
        std::vector<LoggingRule> rules;
        std::shared_mutex mutex;

        std::unordered_set<LogCategory *> categories;

        void updateFilterRules() {
            for (const auto &category : categories) {
                categoryFilter(category);
            }
        }

        static inline LogRegistry *instance() {
            static LogRegistry registry;
            return &registry;
        }
    };

    static void defaultLogCallback(int level, const LogContext &context,
                                   const std::string_view &message) {
        (void) context;

        // The default sink only emits Success and above. Lower levels (Trace/Debug/Information)
        // are dropped here regardless of category filtering, so they need a custom callback to show.
        if (level < Logger::Success) {
            return;
        }

        FILE *out;
        switch (level) {
            case Logger::Success:
                out = stdout;
                break;
            case Logger::Warning:
            case Logger::Critical:
            case Logger::Fatal:
                out = stderr;
                break;
            default:
                assert(false);
                return;
        }
        // NOTE: %s assumes message.data() is null-terminated, which holds for the std::string-backed
        // views the loggers pass in. A view over a non-terminated buffer would over-read.
        fprintf(out, "%s\n", message.data());
    }

    static void defaultLogCategoryFilter(LogCategory *category) {
        // Called by the registry with its lock held; reads LogRegistry::rules without re-locking.
        auto &reg = *LogRegistry::instance();
        std::string_view name = category->name() ? category->name() : "";

        // Baseline: every level enabled (matching the constructor's default), then refine by
        // applying the rules in order so that a later matching rule overrides an earlier one.
        for (int level = Logger::Trace; level <= Logger::Fatal; ++level) {
            category->setLevelEnabled(level, true);
        }
        for (const auto &rule : reg.rules) {
            if (!rule.matchesCategory(name)) {
                continue;
            }
            for (int level = Logger::Trace; level <= Logger::Fatal; ++level) {
                if (rule.appliesToLevel(level)) {
                    category->setLevelEnabled(level, rule.enable);
                }
            }
        }
    }

    void Logger::print(int level, const std::string_view &message) {
        LogRegistry::callback(level, m_ctx, message);
    }

    void Logger::printf(int level, const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        std::string message = lore::vasprintf(fmt, args);
        va_end(args);
        LogRegistry::callback(level, m_ctx, message);
    }

    void Logger::abort() {
        std::abort(); // ###FIXME: robust implementation
    }

    Logger::LogCallback Logger::logCallback() {
        return LogRegistry::callback;
    }

    void Logger::setLogCallback(LogCallback callback) {
        LogRegistry::callback = callback;
    }

    LogCategory::LogCategory(const char *name) : m_name(name) {
        // Set all eight per-level bytes to 1 in one write (one 0x01 byte per levelEnabled[] slot),
        // so every level starts enabled before the filter runs below.
        enabled = 0x0101010101010101ULL;

        auto &reg = *LogRegistry::instance();
        std::unique_lock lock(reg.mutex);
        reg.categories.insert(this);
        reg.categoryFilter(this); // apply any rules already in effect to the new category
    }

    LogCategory::~LogCategory() {
        auto &reg = *LogRegistry::instance();
        std::unique_lock lock(reg.mutex);
        reg.categories.erase(this);
    }

    LogCategory::LogCategoryFilter LogCategory::logFilter() {
        auto &reg = *LogRegistry::instance();
        std::shared_lock lock(reg.mutex);
        return LogRegistry::categoryFilter;
    }

    void LogCategory::setLogFilter(LogCategoryFilter filter) {
        auto &reg = *LogRegistry::instance();
        std::unique_lock lock(reg.mutex);

        if (!filter)
            filter = defaultLogCategoryFilter;

        LogRegistry::categoryFilter = filter;
        reg.updateFilterRules();
    }

    std::string LogCategory::filterRules() {
        auto &reg = *LogRegistry::instance();
        std::shared_lock lock(reg.mutex);
        return reg.filterRules;
    }

    void LogCategory::setFilterRules(std::string rules) {
        auto &reg = *LogRegistry::instance();
        std::unique_lock lock(reg.mutex);
        reg.rules = parseRules(rules);
        reg.filterRules = std::move(rules);
        reg.updateFilterRules();
    }

    LogCategory &LogCategory::defaultCategory() {
        static LogCategory category("default");
        return category;
    }

}

// SPDX-License-Identifier: MIT

#ifndef LORE_SUPPORT_LOGGING_H
#define LORE_SUPPORT_LOGGING_H

#include <lorelei/Support/StringExtras.h>

namespace lore {

    /// LogContext - The call-site information attached to a single log record.
    class LogContext {
    public:
        inline LogContext() noexcept = default;
        inline LogContext(const char *fileName, int lineNumber, const char *functionName,
                          const char *categoryName) noexcept
            : line(lineNumber), file(fileName), function(functionName), category(categoryName) {
        }

        int line = 0;                   ///< source line, or 0 if unknown
        const char *file = nullptr;     ///< source file path
        const char *function = nullptr; ///< enclosing function name
        const char *category = nullptr; ///< originating LogCategory name
    };

    /// Logger - A one-shot emitter for a single log record, bound to a LogContext.
    class LORESUPPORT_EXPORT Logger {
    public:
        /// Severity levels, ordered low to high.
        enum Level {
            Trace = 1,
            Debug,
            Success,
            Information,
            Warning,
            Critical,
            Fatal,
        };

        inline Logger(LogContext context) : m_ctx(std::move(context)) {
        }

        inline Logger(const char *file, int line, const char *function, const char *category)
            : m_ctx(file, line, function, category) {
        }

        /// Formats the arguments with \a formatN() and emits the record at the method's level.
        template <class... Args>
        inline void trace(const std::string_view &format, Args &&...args) {
            print(Trace, formatN(format, std::forward<Args>(args)...));
        }

        template <class... Args>
        inline void debug(const std::string_view &format, Args &&...args) {
            print(Debug, formatN(format, std::forward<Args>(args)...));
        }

        template <class... Args>
        inline void success(const std::string_view &format, Args &&...args) {
            print(Success, formatN(format, std::forward<Args>(args)...));
        }

        template <class... Args>
        inline void info(const std::string_view &format, Args &&...args) {
            print(Information, formatN(format, std::forward<Args>(args)...));
        }

        template <class... Args>
        inline void warning(const std::string_view &format, Args &&...args) {
            print(Warning, formatN(format, std::forward<Args>(args)...));
        }

        template <class... Args>
        inline void critical(const std::string_view &format, Args &&...args) {
            print(Critical, formatN(format, std::forward<Args>(args)...));
        }

        /// Emits the record, then terminates the process via abort().
        template <class... Args>
        [[noreturn]] inline void fatal(const std::string_view &format, Args &&...args) {
            print(Fatal, formatN(format, std::forward<Args>(args)...));
            abort();
        }

        /// Emits the record at a level chosen at run time.
        template <class... Args>
        inline void log(int level, const std::string_view &format, Args &&...args) {
            print(level, formatN(format, std::forward<Args>(args)...));
        }

        /// Hands an already-formatted message to the active LogCallback.
        void print(int level, const std::string_view &message);

        /// Like print(), but builds the message with printf-style formatting.
        void printf(int level, const char *fmt, ...);

        /// Terminates the process. Used by the Fatal path.
        [[noreturn]] static void abort();

    public:
        /// The sink that receives every emitted record: (level, context, message).
        using LogCallback = void (*)(int, const LogContext &, const std::string_view &);

        /// The process-wide sink. The default prints Success and above to stdout/stderr.
        static LogCallback logCallback();
        static void setLogCallback(LogCallback callback);

    protected:
        LogContext m_ctx;
    };

    /// LogCategory - Yet another logging category implementation of Qt \c QLoggingCategory.
    ///
    /// A named logging channel with independently toggleable levels. Each category registers itself
    /// in a global registry on construction (and applies the active filter rules), letting the
    /// logging macros gate output per category and per level. Prefer the lore*/lore*F macros over
    /// calling log() directly.
    class LORESUPPORT_EXPORT LogCategory {
    public:
        /// Registers the category under \c name and applies the current filter rules to it.
        explicit LogCategory(const char *name);
        ~LogCategory();

        /// The category name.
        inline const char *name() const {
            return m_name;
        }
        /// Whether records at \c level (a Logger::Level) are currently emitted.
        inline bool isLevelEnabled(int level) const {
            return levelEnabled[level];
        }
        /// Enables or disables a single level, normally driven by the filter rules.
        inline void setLevelEnabled(int level, bool enabled) {
            levelEnabled[level] = enabled;
        }

        /// A hook that sets a category's enabled levels, run on registration and on rule changes.
        using LogCategoryFilter = void (*)(LogCategory *);

        /// The installed filter. The default one applies filterRules().
        static LogCategoryFilter logFilter();

        /// Replaces the category filter (pass nullptr to restore the default) and re-runs it over
        /// every registered category. A custom filter takes over entirely, so the rules apply only
        /// if it defers to the default.
        static void setLogFilter(LogCategoryFilter filter);

        /// Returns the filter-rules string most recently passed to setFilterRules().
        static std::string filterRules();

        /// Installs Qt-style category filter rules controlling which levels each category emits.
        ///
        /// Rules are separated by newlines or ';', and `#`-prefixed lines are comments. Each rule
        /// reads `category[.level] = true|false`, where:
        ///   - category may carry a single leading and/or trailing `*` wildcard
        ///     (`lore.*`, `*.io`, `*`). Otherwise it matches exactly.
        ///   - level is one of trace/debug/success/info/warning/critical/fatal. Omit it to affect
        ///     every level.
        /// Rules are applied in order over an all-enabled baseline, so a later matching rule
        /// overrides an earlier one. For example:
        /// \code
        ///   *.debug = false       // silence debug everywhere
        ///   lore.io = false       // silence the lore.io category entirely
        ///   lore.io.warning = true // ...except its warnings
        /// \endcode
        void setFilterRules(std::string rules);

        /// The fallback category used by the macros when no user category is in scope.
        static LogCategory &defaultCategory();

        /// Macro entry point: if the compile-time \c Level is enabled, formats the arguments with
        /// formatN() (%1, %2, ...) and emits the record, aborting afterwards when \c Level is
        /// Fatal.
        template <int Level, class... Args>
        void log(const char *fileName, int lineNumber, const char *functionName,
                 const std::string_view &format, Args &&...args) const {
            if (!isLevelEnabled(Level)) {
                return;
            }
            Logger(fileName, lineNumber, functionName, m_name)
                .log(Level, format, std::forward<Args>(args)...);
            if constexpr (Level == Logger::Fatal) {
                Logger::abort();
            }
        }

        /// Like \a log(), but builds the message with printf-style formatting.
        template <int Level, class... Args>
        void logf(const char *fileName, int lineNumber, const char *functionName, const char *fmt,
                  Args &&...args) const {
            if (!isLevelEnabled(Level)) {
                return;
            }
            Logger(fileName, lineNumber, functionName, m_name)
                .printf(Level, fmt, std::forward<Args>(args)...);
            if constexpr (Level == Logger::Fatal) {
                Logger::abort();
            }
        }

        /// Internal: lets the macros resolve an in-scope user category via unqualified lookup.
        inline const LogCategory &__loreGetLogCategory() const {
            return *this;
        }

    protected:
        const char *m_name;

        // Per-level enable flags, indexed by Logger::Level. The union aliases all eight bytes as a
        // single word so every level can be set at once through `enabled` (e.g. the ctor's
        // 0x01..01 turns them all on).
        union {
            bool levelEnabled[8];
            uint64_t enabled;
        };
    };

}

// Fallback for the macros when no LogCategory member is in scope: resolves to the default category.
static inline const lore::LogCategory &__loreGetLogCategory() {
    return lore::LogCategory::defaultCategory();
}

/*!
    \macro loreDebug
    \brief Logs a debug message to a log category.
    \code
        // User category
        lore::LogCategory lc("test");
        lc.setLevelEnabled(lore::Logger::Debug, true);
        lc.loreDebug("This is a debug message");
        lc.loreDebug("This is a debug message with arg: %1", 42);
        lc.loreDebugF("This is a debug message with arg: %d", 42);

        // Default category
        loreDebug("This is a debug message");
        loreDebug("This is a debug message with arg: %1", 42);
        loreDebug("This is a debug message with arg: %d", 42);
    \endcode
*/

// Per-category logging macros. Each resolves the in-scope category through __loreGetLogCategory()
// (the member form for a user category, the free form for the default) and forwards the call site.
// The plain macros use formatN() (%1, %2, ...). The *F variants use printf-style formatting.
#define loreLog(LEVEL, ...)                                                                        \
    __loreGetLogCategory().log<lore::Logger::LEVEL>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define loreTrace(...)    loreLog(Trace, __VA_ARGS__)
#define loreDebug(...)    loreLog(Debug, __VA_ARGS__)
#define loreSuccess(...)  loreLog(Success, __VA_ARGS__)
#define loreInfo(...)     loreLog(Information, __VA_ARGS__)
#define loreWarning(...)  loreLog(Warning, __VA_ARGS__)
#define loreCritical(...) loreLog(Critical, __VA_ARGS__)
#define loreFatal(...)    loreLog(Fatal, __VA_ARGS__)

#define loreLogF(LEVEL, ...)                                                                       \
    __loreGetLogCategory().logf<lore::Logger::LEVEL>(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define loreTraceF(...)    loreLogF(Trace, __VA_ARGS__)
#define loreDebugF(...)    loreLogF(Debug, __VA_ARGS__)
#define loreSuccessF(...)  loreLogF(Success, __VA_ARGS__)
#define loreInfoF(...)     loreLogF(Information, __VA_ARGS__)
#define loreWarningF(...)  loreLogF(Warning, __VA_ARGS__)
#define loreCriticalF(...) loreLogF(Critical, __VA_ARGS__)
#define loreFatalF(...)    loreLogF(Fatal, __VA_ARGS__)

#endif // LORE_SUPPORT_LOGGING_H

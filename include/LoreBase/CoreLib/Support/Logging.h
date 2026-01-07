#ifndef LORE_BASE_CORELIB_LOGGING_H
#define LORE_BASE_CORELIB_LOGGING_H

#include <LoreBase/CoreLib/ADT/StringExtras.h>

namespace lore {

    class LogContext {
    public:
        inline LogContext() noexcept = default;
        inline LogContext(const char *fileName, int lineNumber, const char *functionName,
                          const char *categoryName) noexcept
            : line(lineNumber), file(fileName), function(functionName), category(categoryName) {
        }

        int line = 0;
        const char *file = nullptr;
        const char *function = nullptr;
        const char *category = nullptr;
    };

    class LORECORE_EXPORT Logger {
    public:
        enum Level {
            Trace = 1,
            Debug,
            Success,
            Information,
            Warning,
            Critical,
            Fatal,
        };

        inline Logger(LogContext context) : _context(std::move(context)) {
        }

        inline Logger(const char *file, int line, const char *function, const char *category)
            : _context(file, line, function, category) {
        }

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

        template <class... Args>
        [[noreturn]] inline void fatal(const std::string_view &format, Args &&...args) {
            print(Critical, formatN(format, std::forward<Args>(args)...));
            abort();
        }

        template <class... Args>
        inline void log(int level, const std::string_view &format, Args &&...args) {
            print(level, formatN(format, std::forward<Args>(args)...));
        }

        void print(int level, const std::string_view &message);

        void printf(int level, const char *fmt, ...);

        [[noreturn]] static void abort();

    public:
        using LogCallback = void (*)(int, const LogContext &, const std::string_view &);

        static LogCallback logCallback();
        static void setLogCallback(LogCallback callback);

    protected:
        LogContext _context;
    };

    /// Yet another logging category implementation of Qt QLoggingCategory.
    class LORECORE_EXPORT LogCategory {
    public:
        explicit LogCategory(const char *name);
        ~LogCategory();

        inline const char *name() const {
            return _name;
        }
        inline bool isLevelEnabled(int level) const {
            return levelEnabled[level];
        }
        inline void setLevelEnabled(int level, bool enabled) {
            levelEnabled[level] = enabled;
        }

        using LogCategoryFilter = void (*)(LogCategory *);

        static LogCategoryFilter logFilter();
        static void setLogFilter(LogCategoryFilter filter);

        static std::string filterRules();
        void setFilterRules(std::string rules);

        static LogCategory &defaultCategory();

        template <int Level, class... Args>
        void log(const char *fileName, int lineNumber, const char *functionName,
                 const std::string_view &format, Args &&...args) const {
            if (!isLevelEnabled(Level)) {
                return;
            }
            Logger(fileName, lineNumber, functionName, _name)
                .log(Level, format, std::forward<Args>(args)...);
            if constexpr (Level == Logger::Fatal) {
                Logger::abort();
            }
        }

        template <int Level, class... Args>
        void logf(const char *fileName, int lineNumber, const char *functionName, const char *fmt,
                  Args &&...args) const {
            if (!isLevelEnabled(Level)) {
                return;
            }
            Logger(fileName, lineNumber, functionName, _name)
                .printf(Level, fmt, std::forward<Args>(args)...);
            if constexpr (Level == Logger::Fatal) {
                Logger::abort();
            }
        }

        inline const LogCategory &__loreGetLogCategory() const {
            return *this;
        }

    protected:
        const char *_name;

        union {
            bool levelEnabled[8];
            uint64_t enabled;
        };
    };

}

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

#endif // LORE_BASE_CORELIB_LOGGING_H

#include "HostSyscallServer.h"

#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <memory>
#include <utility>

#include <dlfcn.h>

#include <lorelei/Base/Support/Logging.h>
#include <lorelei/Base/Support/StringExtras.h>

namespace lore {

#ifdef __x86_64__
    static constexpr const char *kHostArch = "x86_64";
#elif defined(__aarch64__)
    static constexpr const char *kHostArch = "aarch64";
#elif defined(__riscv) && __riscv_xlen == 64
    static constexpr const char *kHostArch = "riscv64";
#else
    static constexpr const char *kHostArch = "unknown";
#endif

    static std::filesystem::path inferRootDirFromRuntime() {
        Dl_info info = {};
        if (!dladdr(reinterpret_cast<const void *>(&inferRootDirFromRuntime), &info) ||
            !info.dli_fname) {
            return {};
        }

        auto runtimeDir = std::filesystem::path(info.dli_fname).parent_path();
        auto cursor = runtimeDir;
        while (!cursor.empty()) {
            const auto name = cursor.filename().string();
            if (name == "lib" || name == "lib64") {
                return cursor.parent_path();
            }
            if (!cursor.has_parent_path() || cursor.parent_path() == cursor) {
                break;
            }
            cursor = cursor.parent_path();
        }
        return runtimeDir;
    }

    static std::map<std::string, std::string>
        buildConfigVars(const std::filesystem::path &rootDir,
                        const std::filesystem::path &guestRootDir) {
        std::map<std::string, std::string> vars = {
            {"ROOT",       rootDir.string()                                                  },
            {"GUEST_ROOT", guestRootDir.string()                                             },
            {"HOME",       std::getenv("HOME") ? std::getenv("HOME") : ""                    },
            {"GTL_DIR",    (guestRootDir / "lib" / "x86_64-LoreGTL").string()                },
            {"HTL_DIR",    (rootDir / "lib" / (std::string(kHostArch) + "-LoreHTL")).string()},
            {"ARCH",       kHostArch                                                         },
        };

        if (const char *extraVars = std::getenv("LORELEI_THUNKS_CONFIG_VARIABLES");
            extraVars && *extraVars) {
            for (const auto item : split(std::string_view(extraVars), ";")) {
                const auto eq = item.find('=');
                if (eq == std::string_view::npos) {
                    continue;
                }
                vars[std::string(item.substr(0, eq))] = std::string(item.substr(eq + 1));
            }
        }
        
        return vars;
    }

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s);

    struct LOREHOSTRT_EXPORT HostRuntime {
        int level = Logger::Information;
        FILE *logFile = nullptr;
        Logger::LogCallback defaultLogCallback = nullptr;
        mod::HostSyscallServer server;

        HostRuntime() {
            if (const char *levelStr = std::getenv("LORELEI_HOST_LOG_LEVEL")) {
                level = std::atoi(levelStr);
                if (level == 0) {
                    level = Logger::Information;
                }
            }

            if (const char *logFileStr = std::getenv("LORELEI_HOST_LOG_FILE")) {
                logFile = std::fopen(logFileStr, "w");
            }

            defaultLogCallback = Logger::logCallback();
            Logger::setLogCallback(logCallback);

            std::filesystem::path rootDir;
            if (const char *rootEnv = std::getenv("LORELEI_ROOT")) {
                rootDir = rootEnv;
            } else {
                rootDir = inferRootDirFromRuntime();
            }

            std::filesystem::path guestRootDir;
            if (const char *guestRootEnv = std::getenv("LORELEI_GUEST_ROOT")) {
                guestRootDir = guestRootEnv;
            } else {
                guestRootDir = rootDir;
            }

            std::filesystem::path configPath;
            if (const char *configEnv = std::getenv("LORELEI_THUNK_DATABASE")) {
                configPath = configEnv;
            } else if (!rootDir.empty()) {
                configPath = rootDir / "share" / "lorelei" / "ThunkDB.json";
            } else {
                configPath = std::filesystem::path("share") / "lorelei" / "ThunkDB.json";
            }

            auto config = std::make_unique<ThunkInfoConfig>();
            const auto loaded = config->load(configPath, buildConfigVars(rootDir, guestRootDir));
            if (!loaded) {
                loreWarning("[HRT] %1: Failed to load thunk config", configPath.string());
            }
            server.setThunkConfig(std::move(config));
        }

        ~HostRuntime() {
            if (logFile) {
                std::fclose(logFile);
            }
        }
    };

    LOREHOSTRT_EXPORT HostRuntime runtime_instance;

    LOREHOSTRT_EXPORT thread_local void *thread_last_callback = nullptr;

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s) {
        if (level < runtime_instance.level) {
            return;
        }
        if (runtime_instance.logFile) {
            std::fwrite(s.data(), 1, s.size(), runtime_instance.logFile);
            std::fputc('\n', runtime_instance.logFile);
            return;
        }
        if (runtime_instance.defaultLogCallback) {
            runtime_instance.defaultLogCallback(level, ctx, s);
        }
    }

}

// SPDX-License-Identifier: MIT

#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include <dlfcn.h>

#include <lorelei/Support/Logging.h>
#include <lorelei/Support/StringExtras.h>

#include "HostServer.h"
#include "LogCategory.h"

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

    // The (guestThunkDir, hostThunkDir) pair for a thunk prefix. The prefix mirrors the deployed
    // layout: guest thunks under <prefix>/x86_64/lib/x86_64-LoreGTL, host thunks under
    // <prefix>/lib/<arch>-LoreHTL. This is the one place that encodes the on-disk thunk layout.
    static std::pair<std::filesystem::path, std::filesystem::path>
        thunkDirsForPrefix(const std::filesystem::path &prefix) {
        return {prefix / "x86_64" / "lib" / "x86_64-LoreGTL",
                prefix / "lib" / (std::string(kHostArch) + "-LoreHTL")};
    }

    // Variables for ${...} substitution in a ThunkDB.json: HOME, the host ARCH, and any extra
    // name=value pairs from LORELEI_THUNKS_CONFIG_VARIABLES. GTL_DIR / HTL_DIR are not set here;
    // load() fills them per source with that source's own thunk dirs.
    static std::map<std::string, std::string> buildConfigVars() {
        std::map<std::string, std::string> vars = {
            {"HOME", std::getenv("HOME") ? std::getenv("HOME") : ""},
            {"ARCH", kHostArch                                     },
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
        mod::HostServer server;

        HostRuntime() {
            if (const char *levelStr = std::getenv("LORELEI_HOST_LOG_LEVEL")) {
                level = std::atoi(levelStr);
                // atoi returns 0 for non-numeric or empty input, so treat that as "use the default".
                if (level == 0) {
                    level = Logger::Information;
                }
            }

            if (const char *logFileStr = std::getenv("LORELEI_HOST_LOG_FILE")) {
                logFile = std::fopen(logFileStr, "w");
            }

            defaultLogCallback = Logger::logCallback();
            Logger::setLogCallback(logCallback);

            // Probe a qemu plugin symbol in the default scope: its presence confirms we are loaded
            // inside the patched qemu and gives the host a stable anchor address into the emulator.
            void *emuAddr = dlsym(RTLD_DEFAULT, "qemu_plugin_register_vcpu_syscall_filter_cb");
            if (!emuAddr) {
                log::logger().loreCritical(
                    "failed to find qemu_plugin_register_vcpu_syscall_filter_cb");
                exit(1);
            }
            mod::HostServer::emuAddr = emuAddr;

            // Assemble the thunk sources from LORELEI_THUNK_PATH: a colon-separated list of thunk
            // prefixes, highest priority first. Every prefix mirrors the deployed layout (guest thunks
            // under <prefix>/x86_64, host under <prefix>/lib) and carries its own
            // <prefix>/share/lorelei/ThunkDB.json, so a self-contained thunk pack drops in without a
            // rebuild. The first source to define a thunk name wins. LORELEI_THUNK_NO_AUTOSCAN leaves
            // the sources empty, so only the override JSON applies.
            ThunkDatabase::LoadOptions opts;
            if (std::getenv("LORELEI_THUNK_NO_AUTOSCAN") == nullptr) {
                if (const char *thunkPath = std::getenv("LORELEI_THUNK_PATH");
                    thunkPath && *thunkPath) {
                    for (const auto entry : split(std::string_view(thunkPath), ":")) {
                        if (entry.empty()) {
                            continue;
                        }
                        std::filesystem::path prefix{std::string(entry)};
                        auto dirs = thunkDirsForPrefix(prefix);
                        opts.sources.push_back({std::move(dirs.first), std::move(dirs.second),
                                                prefix / "share" / "lorelei" / "ThunkDB.json"});
                    }
                }
            }

            // An explicit LORELEI_THUNK_DATABASE is a single override JSON layered on top of every
            // source.
            std::filesystem::path overridePath;
            if (const char *configEnv = std::getenv("LORELEI_THUNK_DATABASE")) {
                overridePath = configEnv;
            }

            auto db = std::make_unique<ThunkDatabase>();
            // load() returns false only when a config file that is present cannot be parsed; a missing
            // one is fine, so it is not worth a warning.
            if (!db->load(overridePath, buildConfigVars(), opts)) {
                log::logger().loreWarning("failed to parse a thunk config");
            }
            server.setThunkDatabase(std::move(db));
        }

        ~HostRuntime() {
            if (logFile) {
                std::fclose(logFile);
            }
        }
    };

    LOREHOSTRT_EXPORT HostRuntime runtime_instance;

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s) {
        if (level < runtime_instance.level) {
            return;
        }

        // Render the emitting component's category as a "name: " prefix, skipping the unnamed
        // default. This is the one place runtime records become text: both host-native logs and
        // guest logs forwarded through DS_LogMessage land here, so every line is tagged with its
        // origin and no message string carries its own tag.
        std::string line;
        if (ctx.category && *ctx.category && std::strcmp(ctx.category, "default") != 0) {
            line.reserve(std::strlen(ctx.category) + 2 + s.size());
            line.append(ctx.category).append(": ").append(s);
        } else {
            line.assign(s);
        }

        if (runtime_instance.logFile) {
            std::fwrite(line.data(), 1, line.size(), runtime_instance.logFile);
            std::fputc('\n', runtime_instance.logFile);
            return;
        }
        if (runtime_instance.defaultLogCallback) {
            runtime_instance.defaultLogCallback(level, ctx, line);
        }
    }

}

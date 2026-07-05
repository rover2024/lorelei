// SPDX-License-Identifier: MIT

#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <memory>
#include <utility>

#include <dlfcn.h>

#include <lorelei/Support/Logging.h>
#include <lorelei/Support/StringExtras.h>

#include "HostServer.h"

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

        // Walk up from the runtime's directory to the install prefix: the first "lib"/"lib64"
        // ancestor's parent is the root. Fall back to the runtime directory if none is found.
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

    // The (guestThunkDir, hostThunkDir) pair for a thunk prefix. The prefix mirrors the deployed
    // layout: guest thunks under <prefix>/x86_64/lib/x86_64-LoreGTL, host thunks under
    // <prefix>/lib/<arch>-LoreHTL. This is the one place that encodes the on-disk thunk layout, shared
    // by the JSON vars and the scan path.
    static std::pair<std::filesystem::path, std::filesystem::path>
        thunkDirsForPrefix(const std::filesystem::path &prefix) {
        return {prefix / "x86_64" / "lib" / "x86_64-LoreGTL",
                prefix / "lib" / (std::string(kHostArch) + "-LoreHTL")};
    }

    static std::map<std::string, std::string>
        buildConfigVars(const std::filesystem::path &rootDir) {
        auto [gtlDir, htlDir] = thunkDirsForPrefix(rootDir);
        std::map<std::string, std::string> vars = {
            {"ROOT",    rootDir.string()},
            {"HOME",    std::getenv("HOME") ? std::getenv("HOME") : ""},
            {"GTL_DIR", gtlDir.string() },
            {"HTL_DIR", htlDir.string() },
            {"ARCH",    kHostArch       },
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
                loreCritical("[HRT] Failed to find qemu_plugin_register_vcpu_syscall_filter_cb\n");
                exit(1);
            }
            mod::HostServer::emuAddr = emuAddr;

            std::filesystem::path rootDir;
            if (const char *rootEnv = std::getenv("LORELEI_ROOT")) {
                rootDir = rootEnv;
            } else {
                rootDir = inferRootDirFromRuntime();
            }

            std::filesystem::path configPath;
            if (const char *configEnv = std::getenv("LORELEI_THUNK_DATABASE")) {
                configPath = configEnv;
            } else if (!rootDir.empty()) {
                configPath = rootDir / "share" / "lorelei" / "ThunkDB.json";
            } else {
                configPath = std::filesystem::path("share") / "lorelei" / "ThunkDB.json";
            }

            // Assemble the thunk search path from thunk prefixes: the LORELEI_THUNK_PATH entries
            // (colon-separated), scanned first and in order, then the base tree (LORELEI_ROOT) as the
            // final, lowest-priority fallback. Every prefix mirrors the deployed layout, so its guest
            // thunks are always under <prefix>/x86_64; there is no separate guest-root knob. The first
            // pair to define a thunk name wins, and the JSON at configPath is layered on top as
            // overrides. LORELEI_THUNK_NO_AUTOSCAN leaves the search path empty, so only the JSON applies.
            ThunkDatabase::LoadOptions opts;
            if (std::getenv("LORELEI_THUNK_NO_AUTOSCAN") == nullptr) {
                if (const char *thunkPath = std::getenv("LORELEI_THUNK_PATH");
                    thunkPath && *thunkPath) {
                    for (const auto entry : split(std::string_view(thunkPath), ":")) {
                        if (entry.empty()) {
                            continue;
                        }
                        opts.scanDirs.push_back(thunkDirsForPrefix(std::string(entry)));
                    }
                }
                opts.scanDirs.push_back(thunkDirsForPrefix(rootDir));
            }

            auto db = std::make_unique<ThunkDatabase>();
            // load() returns false only for a config file that exists but cannot be parsed; a missing
            // file is fine (the scan still runs), so it is not worth a warning.
            if (!db->load(configPath, buildConfigVars(rootDir), opts)) {
                loreWarning("[HRT] %1: Failed to parse thunk config", configPath.string());
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

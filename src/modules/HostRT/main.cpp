// SPDX-License-Identifier: MIT

#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <string>

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

    // Variables for ${...} substitution in a ThunkDB.json: HOME, the host ARCH, and any extra
    // name=value pairs from LORELEI_THUNKS_CONFIG_VARIABLES. GTL_DIR / HTL_DIR are not set here;
    // each source fills them with its own thunk dirs when its JSON is loaded.
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

            // Thunk discovery follows each thunk's own on-disk location: a guest thunk reports its
            // resolved path, and the host derives its host thunk around it (see
            // HostServer::resolveForwardThunk). LORELEI_THUNK_NO_AUTODISCOVER turns that off, leaving
            // only the override JSON. LORELEI_THUNK_DATABASE names that override JSON, layered on top.
            std::filesystem::path overridePath;
            if (const char *configEnv = std::getenv("LORELEI_THUNK_DATABASE")) {
                overridePath = configEnv;
            }
            const bool autoDiscover = std::getenv("LORELEI_THUNK_NO_AUTODISCOVER") == nullptr;
            server.configureThunkDiscovery(buildConfigVars(), overridePath, kHostArch, autoDiscover);
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

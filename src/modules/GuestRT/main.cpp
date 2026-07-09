// SPDX-License-Identifier: MIT

#include <dlfcn.h>

#include <cstdio>
#include <cstdlib>

#include <lorelei/Support/Logging.h>

#include "GuestClient.h"
#include "LogCategory.h"

namespace lore {

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s);

    // Report a bootstrap failure with the host-side dlerror() and abort. getLibraryError talks to
    // the plugin directly (not through the common entry), so it is usable this early, before the
    // common entry is resolved. The log callback is not installed yet either, so this prints to
    // stderr directly, tagged with the same guest-rt category the host sink would render.
    [[noreturn]] static void abortHostError(const char *what) {
        const char *err = mod::GuestClient::getLibraryError();
        std::fprintf(stderr, "%s: %s: %s\n", log::logger().name(), what,
                     err ? err : "unknown error");
        std::abort();
    }

    struct LOREGUESTRT_EXPORT GuestRuntime {
        int level = Logger::Information;
        mod::GuestClient client;

        GuestRuntime() {
            void *hostRuntimeHandle = mod::GuestClient::loadLibrary("libLoreHostRT.so", RTLD_NOW);
            if (!hostRuntimeHandle) {
                abortHostError("failed to load host runtime");
            }

            // Resolve the host runtime's common entry. Every InvokeProc-based request
            // (logMessage, invokeFunction, getModulePath, getThunkInfo, ...) is routed
            // through it, so it must be set before any of those are issued, including
            // before the log callback below, which forwards through it.
            void *commonHostEntry =
                mod::GuestClient::getProcAddress(hostRuntimeHandle, "LoreCommonHostEntry");
            if (!commonHostEntry) {
                abortHostError("failed to resolve host common entry");
            }
            mod::GuestClient::setCommonHostEntry(commonHostEntry);

            if (const char *levelStr = std::getenv("LORELEI_GUEST_LOG_LEVEL")) {
                level = std::atoi(levelStr);
                if (level == 0) {
                    level = Logger::Information;
                }
            }
            Logger::setLogCallback(logCallback);
        }
    };

    LOREGUESTRT_EXPORT GuestRuntime runtime_instance;

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s) {
        if (level >= runtime_instance.level) {
            runtime_instance.client.logMessage(level, ctx, s.data());
        }
    }

}

#include <dlfcn.h>

#include <cstdio>
#include <cstdlib>

#include <lorelei/Support/Logging.h>

#include "GuestClient.h"

namespace lore {

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s);

    struct LOREGUESTRT_EXPORT GuestRuntime {
        int level = Logger::Information;
        mod::GuestClient client;

        GuestRuntime() {
            void *hostRuntimeHandle = mod::GuestClient::loadLibrary("libLoreHostRT.so", RTLD_NOW);
            if (!hostRuntimeHandle) {
                std::fputs("[GRT] failed to load host runtime\n", stderr);
                std::abort();
            }

            // Resolve the host runtime's common entry. Every InvokeProc-based request
            // (logMessage, invokeFunction, getModulePath, getThunkInfo, ...) is routed
            // through it, so it must be set before any of those are issued -- including
            // before the log callback below, which forwards through it.
            void *commonHostEntry =
                mod::GuestClient::getProcAddress(hostRuntimeHandle, "LoreCommonHostEntry");
            if (!commonHostEntry) {
                std::fputs("[GRT] failed to resolve host common entry\n", stderr);
                std::abort();
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

    LOREGUESTRT_EXPORT thread_local void *thread_last_callback = nullptr;

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s) {
        if (level >= runtime_instance.level) {
            runtime_instance.client.logMessage(level, ctx, s.data());
        }
    }

}

#include "GuestSyscallBridge.h"

#include <cstdlib>

#include <stdcorelib/support/logging.h>

namespace lore {

    static void logCallback(int level, const stdc::LogContext &ctx, const std::string_view &s);

    struct LOREGUESTRT_EXPORT GuestRuntime {
        int level = stdc::Logger::Information;

        // The syscall bridge instance
        GuestSyscallBridge bridge;

        GuestRuntime() {
            if (bridge.checkHealth() != 0) {
                fprintf(stderr, "[GRT] lorelei check health failed\n");
                std::abort();
            }

            auto level_str = std::getenv("LORELEI_GUEST_LOG_LEVEL");
            if (level_str) {
                level = std::atoi(level_str);
                if (level == 0) {
                    level = stdc::Logger::Information;
                }
            }
            stdc::Logger::setLogCallback(logCallback);
        }

        ~GuestRuntime() {
            // ?
        }
    };

    LOREGUESTRT_EXPORT GuestRuntime runtime_instance;

    LOREGUESTRT_EXPORT thread_local void *thread_last_gcb;

    static void logCallback(int level, const stdc::LogContext &ctx, const std::string_view &s) {
        if (level >= runtime_instance.level) {
            runtime_instance.bridge.logMessage(level, &ctx, s.data());
        }
    }

}
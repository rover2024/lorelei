#include "GuestSyscallClient.h"

#include <cstdio>
#include <cstdlib>

#include <lorelei/Base/Support/Logging.h>

namespace lore {

    static void logCallback(int level, const LogContext &ctx, const std::string_view &s);

    struct LOREGUESTRT_EXPORT GuestRuntime {
        int level = Logger::Information;
        mod::GuestSyscallClient client;

        GuestRuntime() {
            const auto arch = mod::GuestSyscallClient::getHostAttribute("host_arch");
            if (!arch.data || arch.size == 0) {
                std::fputs("[GRT] failed to communicate with host runtime\n", stderr);
                std::abort();
            }

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

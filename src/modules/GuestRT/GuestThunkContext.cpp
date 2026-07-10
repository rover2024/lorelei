// SPDX-License-Identifier: MIT

#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <cstdio>
#include <cstdint>
#include <map>
#include <string>

#include <lorelei/Support/Logging.h>

#include <NextLibrary.h>

#include "GuestClient.h"
#include "LogCategory.h"

#define LORE_GUESTRT_USE_PROC_MAPS_FOR_SELF_PATH 1

namespace lore::mod {

#if LORE_GUESTRT_USE_PROC_MAPS_FOR_SELF_PATH
    namespace {

        std::string findModulePathByAddress(const void *addr) {
            auto target = reinterpret_cast<uintptr_t>(addr);
            std::FILE *maps = std::fopen("/proc/self/maps", "r");
            if (!maps) {
                return {};
            }

            char line[4096];
            while (std::fgets(line, sizeof(line), maps)) {
                unsigned long long lo = 0;
                unsigned long long hi = 0;
                char perms[8] = {};
                char path[3072] = {};
                int n = std::sscanf(line, "%llx-%llx %7s %*s %*s %*s %3071[^\n]", &lo,
                                    &hi, perms, path);
                if (n >= 3 && target >= lo && target < hi) {
                    std::fclose(maps);
                    return n == 4 ? std::string(path) : std::string();
                }
            }

            std::fclose(maps);
            return {};
        }

    }
#endif

    GuestThunkContext::~GuestThunkContext() {
        if (m_htlHandle) {
            std::ignore = GuestClient::freeLibrary(m_htlHandle);
        }
    }

    void GuestThunkContext::initialize() {
        // Recover this thunk library's own path from the address of its static context, which lives
        // inside the library image.
#if LORE_GUESTRT_USE_PROC_MAPS_FOR_SELF_PATH
        std::string modulePathStorage = findModulePathByAddress(m_staticThunkContext);
        if (modulePathStorage.empty()) {
            log::logger().loreCriticalF("failed to get thunk library name of address %p",
                                        (void *) m_staticThunkContext);
            std::abort();
        }
        const char *modulePath = modulePathStorage.c_str();
#else
        Dl_info selfInfo;
        if (!dladdr(m_staticThunkContext, &selfInfo)) {
            log::logger().loreCriticalF("failed to get thunk library name of address %p",
                                        (void *) m_staticThunkContext);
            std::abort();
        }
        const char *modulePath = selfInfo.dli_fname;
#endif

        // Pick the host thunk library (HTL) to load. The database wins if it names one (a JSON entry);
        // otherwise the path baked into this thunk; otherwise the lib<name>_HTL.so name convention.
        std::string next;
        if (auto info = GuestClient::getThunkInfo(modulePath, false);
            info.forward && info.forward->hostThunk && *info.forward->hostThunk) {
            next = info.forward->hostThunk;
        } else if (const char *baked = m_staticThunkContext->nextLibraryPath; baked && *baked) {
            next = baked;
        } else {
            next = utils::nextLibraryByName(modulePath, /*hostThunk=*/false);
        }

        std::string htlPath = utils::resolveNextLibrary(next, modulePath);
        m_htlHandle = GuestClient::loadLibrary(htlPath.c_str(), RTLD_NOW);
        if (!m_htlHandle) {
            const char *err = GuestClient::getLibraryError();
            log::logger().loreCriticalF("%s: failed to load HTL (%s)", htlPath.c_str(),
                                        err ? err : "unknown error");
            std::abort();
        }

        // Hand the static context across to the HTL so both sides share the same proc table.
        void *exchangeFunc = GuestClient::getProcAddress(m_htlHandle, "LoreExchangeContext");
        if (!exchangeFunc) {
            const char *err = GuestClient::getLibraryError();
            log::logger().loreCriticalF("%s: failed to get init proc (%s)", htlPath.c_str(),
                                        err ? err : "unknown error");
            std::abort();
        }
        void *args[] = {
            &m_staticThunkContext,
        };
        GuestClient::invokeFormat(exchangeFunc, "v_p", args, nullptr);
    }

}

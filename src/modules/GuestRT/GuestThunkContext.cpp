// SPDX-License-Identifier: MIT

#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <map>
#include <string>

#include <lorelei/Support/Logging.h>

#include "GuestClient.h"
#include "LogCategory.h"

namespace lore::mod {

    GuestThunkContext::~GuestThunkContext() {
        if (m_htlHandle) {
            std::ignore = GuestClient::freeLibrary(m_htlHandle);
        }
    }

    void GuestThunkContext::initialize() {
        // Recover this thunk library's own path from the address of its static context, which lives
        // inside the library image.
        Dl_info selfInfo;
        if (!dladdr(m_staticThunkContext, &selfInfo)) {
            log::logger().loreCriticalF("failed to get thunk library name of address %p",
                                        (void *) m_staticThunkContext);
            std::abort();
        }
        const char *modulePath = selfInfo.dli_fname;

        // Find the matching host thunk library (HTL). A build-time hostThunkPath baked into the thunk
        // locates the HTL directly (absolute, or relative to this thunk's own directory); otherwise the
        // host derives it from the deployed layout.
        std::string htlPath;
        if (const char *baked = m_staticThunkContext->hostThunkPath; baked && *baked) {
            if (baked[0] == '/') {
                htlPath = baked;
            } else {
                std::string dir(modulePath);
                auto slash = dir.find_last_of('/');
                htlPath = (slash == std::string::npos ? std::string(".") : dir.substr(0, slash));
                htlPath += '/';
                htlPath += baked;
            }
        } else {
            auto info = GuestClient::getThunkInfo(modulePath, false);
            if (!info.forward) {
                log::logger().loreCritical("%1: failed to get thunk info", modulePath);
                std::abort();
            }
            htlPath = info.forward->hostThunk;
        }
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

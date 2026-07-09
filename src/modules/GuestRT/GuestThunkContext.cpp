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
            log::logger().loreCritical("failed to get thunk library name");
            std::abort();
        }
        const char *modulePath = selfInfo.dli_fname;

        // Look up the matching host thunk library (HTL) and load it host-side.
        auto info = GuestClient::getThunkInfo(modulePath, false);
        if (!info.forward) {
            log::logger().loreCritical("%1: failed to get thunk info", modulePath);
            std::abort();
        }
        const char *htlPath = info.forward->hostThunk;
        m_htlHandle = GuestClient::loadLibrary(htlPath, RTLD_NOW);
        if (!m_htlHandle) {
            const char *err = GuestClient::getLibraryError();
            log::logger().loreCritical("%1: failed to load HTL (%2)", htlPath,
                                       err ? err : "unknown error");
            std::abort();
        }

        // Hand the static context across to the HTL so both sides share the same proc table.
        void *exchangeFunc = GuestClient::getProcAddress(m_htlHandle, "LoreExchangeContext");
        if (!exchangeFunc) {
            const char *err = GuestClient::getLibraryError();
            log::logger().loreCritical("%1: failed to get init proc (%2)", htlPath,
                         err ? err : "unknown error");
            std::abort();
        }
        void *args[] = {
            &m_staticThunkContext,
        };
        GuestClient::invokeFormat(exchangeFunc, "v_p", args, nullptr);
    }

}

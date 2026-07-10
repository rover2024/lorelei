// SPDX-License-Identifier: MIT

#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <map>
#include <string>

#include <lorelei/Support/Logging.h>

#include <NextLibrary.h>

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

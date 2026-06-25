#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <map>
#include <string>

#include <lorelei/Support/Logging.h>

#include "GuestClient.h"

namespace lore::mod {

    GuestThunkContext::~GuestThunkContext() {
        if (m_htlHandle) {
            std::ignore = GuestClient::freeLibrary(m_htlHandle);
        }
    }

    void GuestThunkContext::initialize() {
        /// STEP: get thunk name
        const char *modulePath = nullptr;
        Dl_info selfInfo;
        if (!dladdr(m_staticThunkContext, &selfInfo)) {
            loreCritical("[GTL] failed to get thunk library name");
            std::abort();
        }
        modulePath = selfInfo.dli_fname;

        /// STEP: load host thunk library
        const char *htlPath;
        {
            auto info = GuestClient::getThunkInfo(modulePath, false);
            if (!info.forward) {
                loreCritical("[GTL] %1: failed to get thunk info", modulePath);
                std::abort();
            }
            htlPath = info.forward->hostThunk;
            m_htlHandle = GuestClient::loadLibrary(htlPath, RTLD_NOW);
        }

        if (!m_htlHandle) {
            loreCritical("[GTL] %1: failed to load HTL", htlPath);
            std::abort();
        }

        /// STEP: exchange context
        {
            void *exchangeFunc =
                GuestClient::getProcAddress(m_htlHandle, "LoreExchangeContext");
            if (!exchangeFunc) {
                loreCritical("[GTL] %1: failed to get init proc", htlPath);
                std::abort();
            }
            void *args[] = {
                &m_staticThunkContext,
            };
            GuestClient::invokeFormat(exchangeFunc, "v_p", args, nullptr);
        }
    }

}

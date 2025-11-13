#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <cstring>

#include <stdcorelib/support/logging.h>

#include "GuestClient.h"

namespace lore {

    GuestThunkContext::~GuestThunkContext() {
    }

    void GuestThunkContext::initialize() {
        GuestClient *client = GuestClient::instance();

        /// STEP: load host thunk library
        {
            auto info = client->getThunkInfo(_moduleName, false);
            if (!info.forward) {
                stdcCritical("[GTL] %1: failed to get thunk info", _moduleName);
                std::abort();
            }
            _handle = client->loadLibrary(info.forward->hostThunk, RTLD_NOW);
        }

        if (!_handle) {
            stdcCritical("[GTL] %1: failed to load HTL", _moduleName);
            std::abort();
        }

        /// STEPS: exchange thunks
        {
            void *init = client->getProcAddress(_handle, "LORETHUNK_exchange");
            if (!init) {
                stdcCritical("[GTL] %1: failed to get init proc", _moduleName);
                std::abort();
            }
            void *args[] = {
                _procInfoCtx,
            };
            client->invokeFormat("v_p", args, nullptr);
        }
    }

}
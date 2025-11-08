#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <cstring>

#include <stdcorelib/support/logging.h>

#include "GuestClient.h"

namespace lore {

    GuestThunkContext::~GuestThunkContext() {
    }

    void GuestThunkContext::initThunks() {
        GuestClient *client = GuestClient::instance();

        /// STEP: load host thunk library
        {
            auto info = client->getThunkInfo(_moduleName, false);
            if (!info.forward) {
                stdcCritical("[GTL] %1: failed to get thunk info", _moduleName);
            }
            _handle = client->loadLibrary(info.forward->hostThunk.c_str(), RTLD_NOW);
        }

        if (!_handle) {
            stdcCritical("[GTL] %1: failed to load HTL", _moduleName);
            std::abort();
        }

        /// STEPS: exchange callbacks
        {
            void *init = client->getProcAddress(_handle, "__LORETHUNK_initialize");
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
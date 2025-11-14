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
        const char *path;
        {
            auto info = client->getThunkInfo(_moduleName, false);
            if (!info.forward) {
                stdcCritical("[GTL] %1: failed to get thunk info", _moduleName);
                std::abort();
            }
            path = info.forward->hostThunk;
            _handle = client->loadLibrary(path, RTLD_NOW);
        }

        if (!_handle) {
            stdcCritical("[GTL] %1: failed to load HTL", path);
            std::abort();
        }

        /// STEP: initialize GTL library functions
        {
            auto &entries = _procInfoCtx->libEntries;
            for (size_t i = 0; i < entries.size; ++i) {
                auto &entry = entries.arr[i];
                entry.addr = dlsym(nullptr, entry.name);
                if (!entry.addr) {
                    stdcCritical("[GTL] %1: failed to get proc address %2", path, entry.name);
                    std::abort();
                }
            }
        }

        /// STEPS: exchange thunks
        {
            void *init = client->getProcAddress(_handle, "LORETHUNK_exchange");
            if (!init) {
                stdcCritical("[GTL] %1: failed to get init proc", path);
                std::abort();
            }
            void *args[] = {
                &_procInfoCtx,
            };
            client->invokeFormat(init, "v_p", args, nullptr);
        }
    }

}
#include "GuestThunkContext.h"

#include <dlfcn.h>

#include <cstring>

#include <stdcorelib/support/logging.h>

#include "GuestSyscallBridge.h"

namespace lore {

    GuestThunkContext::~GuestThunkContext() {
    }

    void GuestThunkContext::initThunks() {
        GuestSyscallBridge *bridge = GuestSyscallBridge::instance();

        /// STEP: load host thunk library
        {
            auto info = bridge->getThunkInfo(_moduleName, false);
            if (!info.forward) {
                stdcCritical("[GTL] %1: failed to get thunk info", _moduleName);
            }
            _handle = bridge->loadLibrary(info.forward->hostThunk.c_str(), RTLD_NOW);
        }

        if (!_handle) {
            stdcCritical("[GTL] %1: failed to load HTL", _moduleName);
            std::abort();
        }

        /// STEPS: exchange callbacks
        {
            void *init = bridge->getProcAddress(_handle, "__LORETHUNK_initialize");
            if (!init) {
                stdcCritical("[GTL] %1: failed to get init proc", _moduleName);
                std::abort();
            }
            void *args[] = {
                _procInfoCtx,
            };
            bridge->invokeFormat("v_p", args, nullptr);
        }
    }

}
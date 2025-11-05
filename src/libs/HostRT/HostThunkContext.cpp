#include "HostThunkContext.h"

#include <dlfcn.h>

#include <stdcorelib/support/logging.h>

#include "HostSyscallDispatcher.h"

namespace lore {

    HostThunkContext::~HostThunkContext() {
    }

    void HostThunkContext::initThunks() {
        HostSyscallDispatcher *dispatcher = HostSyscallDispatcher::instance();

        /// STEP: load host library
        {
            auto info = dispatcher->config().forwardThunk(_moduleName);
            if (!info.first) {
                stdcCritical("[HTL] %1: failed to get thunk info", _moduleName);
            }
            _handle = dlopen(info.second.hostLibrary.c_str(), RTLD_NOW);
        }

        if (!_handle) {
            stdcCritical("[HTL] %1: failed to load host library", _moduleName);
            std::abort();
        }
    }


}
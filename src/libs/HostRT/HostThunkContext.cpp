#include "HostThunkContext.h"

#include <dlfcn.h>

#include <stdcorelib/support/logging.h>

#include "HostServer.h"

namespace lore {

    HostThunkContext::~HostThunkContext() {
    }

    void HostThunkContext::initialize() {
        HostServer *server = HostServer::instance();

        /// STEP: load host library
        {
            auto info = server->config().forwardThunk(_moduleName);
            if (!info) {
                stdcCritical("[HTL] %1: failed to get thunk info", _moduleName);
                std::abort();
            }
            _handle = dlopen(info->get().hostLibrary.c_str(), RTLD_NOW);
        }

        if (!_handle) {
            stdcCritical("[HTL] %1: failed to load host library", _moduleName);
            std::abort();
        }

        /// STEP: initialize real library functions
        {
            auto &entries = _procInfoCtx->libEntries;
            for (size_t i = 0; i < entries.size; ++i) {
                auto &entry = entries.arr[i];
                entry.addr = dlsym(_handle, entry.name);
                if (!entry.addr) {
                    stdcCritical("[HTL] %1: failed to get proc address %2", _moduleName,
                                 entry.name);
                    std::abort();
                }
            }
        }
    }


}
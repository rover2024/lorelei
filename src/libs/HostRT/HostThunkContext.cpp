#include "HostThunkContext.h"

#include <dlfcn.h>

#include <stdcorelib/support/logging.h>

#include "HostServer.h"
#include "MultiThreading.h"

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

        /// STEP: initialize host library context
        {
            struct LoreThunk_HLContext {
                void *AddressBoundary;
                void (*HrtSetThreadCallback)(void *callback);
                decltype(&LOREHOSTRT_pthread_create) HrtPThreadCreate;
                decltype(&LOREHOSTRT_pthread_exit) HrtPThreadExit;
                void *CFIs[];
            };
            auto ctx = (LoreThunk_HLContext *) dlsym(_handle, "s_LoreThunk_HLContext");
            if (ctx) {
                ctx->AddressBoundary = _procInfoCtx->emuAddr;
                ctx->HrtSetThreadCallback = [](void *callback) {
                    extern thread_local void *thread_last_callback;
                    thread_last_callback = callback;
                };
                ctx->HrtPThreadCreate = LOREHOSTRT_pthread_create;
                ctx->HrtPThreadExit = LOREHOSTRT_pthread_exit;

                // initialize CFIs
                auto &CFIs = _procInfoCtx->htpEntries[CProcKind_GuestCallback];
                for (int i = 0; i < CFIs.size; ++i) {
                    ctx->CFIs[i] = CFIs.arr[i].addr;
                }
            } else {
                stdcWarning("[HTL] %1: host library is not a lorelei-patched library", _moduleName);
            }
        }
    }

}
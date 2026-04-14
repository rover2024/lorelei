#include "HostThunkContext.h"

#include <dlfcn.h>

#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <strings.h>

#include <lorelei/Base/Support/Logging.h>
#include <lorelei/Base/Support/StringExtras.h>

#include "HostSyscallServer.h"

namespace lore {

    extern thread_local void *thread_last_callback;

}

namespace lore::mod {

    namespace {

        // not used
        struct LoreRuntimeContext {
            void *pthread_create;
            void *pthread_exit;
        };

        struct LoreFileContext {
            LoreRuntimeContext *runtimeContext;
            void *emuAddr;
            void (*setThreadCallback)(void *);
            size_t numCCGs;
            CStaticCallCheckGuardInfo *CCGs;
            size_t numFDGs;
            CStaticFunctionDecayGuardInfo *FDGs;
        };

        using PFN_GetFileContext = LoreFileContext *(*) ();

        static const char *pathGetName(const char *path) {
            const char *slashPos = std::strrchr(path, '/');
            if (!slashPos) {
                return path;
            }
            return slashPos + 1;
        }

        static std::string getLibraryName(const char *path) {
            const char *start = pathGetName(path);
            const char *end = nullptr;

            const int len = std::strlen(path);
            for (const char *p = path + len - 3; p >= start; --p) {
                if (strncasecmp(p, ".so", 3) == 0) {
                    end = p;
                    break;
                }
            }

            if (!end) {
                end = path + len;
            }
            return {start, static_cast<size_t>(end - start)};
        }

        static std::string normalizeThunkName(const char *modulePath) {
            auto name = getLibraryName(modulePath ? modulePath : "");
            if (str::ends_with(name, "_HTL")) {
                name.resize(name.size() - 4);
            }
            return name;
        }

        static void setThreadCallback(void *callback) {
            thread_last_callback = callback;
        }

    }

    HostThunkContext::~HostThunkContext() {
        if (m_hostLibraryHandle) {
            std::ignore = dlclose(m_hostLibraryHandle);
        }
    }

    void HostThunkContext::initialize() {
        assert(m_staticContext != nullptr);

        m_staticContext->emuAddr = HostSyscallServer::emuAddr;

        /// STEP: get thunk name
        const char *modulePath = nullptr;
        Dl_info selfInfo = {};
        if (!dladdr(m_staticContext, &selfInfo)) {
            loreCritical("[HTL] failed to get current thunk module path");
            std::abort();
        }
        modulePath = selfInfo.dli_fname;

        const auto *server = HostSyscallServer::instance();
        assert(server != nullptr);
        const auto *config = server->thunkConfig();
        assert(config != nullptr);

        const auto thunkName = normalizeThunkName(modulePath);
        const auto *forward = config->forwardThunk(thunkName);
        if (!forward) {
            loreCritical("[HTL] %1: failed to resolve forward thunk info", modulePath);
            std::abort();
        }

        /// STEP: load host library
        m_hostLibraryHandle = dlopen(forward->hostLibrary.c_str(), RTLD_NOW);
        if (!m_hostLibraryHandle) {
            loreCritical("[HTL] %1: failed to load host library (%2)", modulePath,
                         forward->hostLibrary);
            std::abort();
        }

        /// STEP: resolve host library symbols
#ifndef LORE_THUNK_CONFIG_HOST_FUNCTION_STATIC_LINK
        // Resolve host-side real functions used by ProcFn<GuestToHost, Exec>.
        for (size_t i = 0; i < m_staticContext->thisProcs.size; ++i) {
            auto &entry = m_staticContext->thisProcs.arr[i];
            assert(entry.name != nullptr);
            entry.addr = dlsym(m_hostLibraryHandle, entry.name);
            if (!entry.addr) {
                loreCritical("[HTL] %1: failed to resolve symbol %2 from host library", modulePath,
                             entry.name);
                std::abort();
            }
        }
#endif

        /// STEP: initialize host library context
        auto getFileContext = reinterpret_cast<PFN_GetFileContext>(
            dlsym(m_hostLibraryHandle, "__Lore_GetFileContext"));
        if (!getFileContext) {
            loreWarning(
                "[HTL] %1: host library is not a HLR-patched library, skip HLR runtime wiring",
                modulePath);
            return;
        }

        auto fileCtx = getFileContext();
        if (!fileCtx) {
            loreWarning("[HTL] %1: __Lore_GetFileContext returned null", modulePath);
            return;
        }

        fileCtx->emuAddr = HostSyscallServer::emuAddr;
        fileCtx->setThreadCallback = setThreadCallback;
        if (fileCtx->runtimeContext) {
            fileCtx->runtimeContext->pthread_create = nullptr;
            fileCtx->runtimeContext->pthread_exit = nullptr;
        }

        std::map<std::string, void *> hostToGuestCallbackThunkBySignature;
        {
            auto callbackEntries =
                m_staticContext->hostProcs[CProcKind_Callback][CProcDirection_HostToGuest];
            for (size_t i = 0; i < callbackEntries.size; ++i) {
                auto &entry = callbackEntries.arr[i];
                assert(entry.name != nullptr);
                assert(entry.addr != nullptr);
                hostToGuestCallbackThunkBySignature[entry.name] = entry.addr;
            }
        }

        /// STEP: fill call check guard list
        if (fileCtx->CCGs) {
            m_staticContext->numCCGs = fileCtx->numCCGs;
            m_staticContext->CCGs = fileCtx->CCGs;
            for (size_t i = 0; i < fileCtx->numCCGs; ++i) {
                auto &ccgInfo = fileCtx->CCGs[i];
                assert(ccgInfo.signature != nullptr);
                assert(ccgInfo.pTramp != nullptr);

                auto it = hostToGuestCallbackThunkBySignature.find(ccgInfo.signature);
                if (it == hostToGuestCallbackThunkBySignature.end()) {
                    loreCritical("[HTL] %1: failed to find host-to-guest callback entry for CCG "
                                 "signature \"%2\"",
                                 modulePath, ccgInfo.signature);
                    std::abort();
                }
                *ccgInfo.pTramp = it->second;
            }
        }

        // Function decay guard list will be filled in GuestThunkContext::initialize()
        if (fileCtx->FDGs) {
            m_staticContext->numFDGs = fileCtx->numFDGs;
            m_staticContext->FDGs = fileCtx->FDGs;
        }
    }

}

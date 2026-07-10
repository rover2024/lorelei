// SPDX-License-Identifier: MIT

#include "HostThunkContext.h"

#include <dlfcn.h>

#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <strings.h>

#include <lorelei/Support/Logging.h>
#include <lorelei/Support/StringExtras.h>

#include <NextLibrary.h>

#include "HostServer.h"
#include "LogCategory.h"

namespace lore {

    extern "C" __thread __attribute__((tls_model("initial-exec"))) void *thread_last_callback;

}

namespace lore::mod {

    namespace {

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

            // Strip the rightmost ".so" (case-insensitive) so e.g. "libfoo.so.1" yields "libfoo".
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

    }

    HostThunkContext::~HostThunkContext() {
        if (m_hostLibraryHandle) {
            std::ignore = dlclose(m_hostLibraryHandle);
        }
    }

    void HostThunkContext::initialize() {
        assert(m_staticThunkContext != nullptr);

        m_staticThunkContext->emuAddr = HostServer::emuAddr;

        /// STEP: get thunk name
        // Derive this HTL's own module path from the static context's address. The thunk name keys
        // its forward-thunk lookup in the database.
        Dl_info selfInfo = {};
        if (!dladdr(m_staticThunkContext, &selfInfo)) {
            log::logger().loreCritical("failed to get current thunk module path");
            std::abort();
        }
        const char *modulePath = selfInfo.dli_fname;

        // With AUTO_LINK the real library's symbols were folded in at link time, so there is nothing to
        // load or resolve here and no database entry is needed.
        if (m_staticThunkContext->autoLink) {
            return;
        }

        // Pick the real library to load. The database wins if it names one; otherwise the path baked
        // into this thunk; otherwise the lib<name>.so name convention.
        const auto *server = HostServer::instance();
        assert(server != nullptr);
        const auto *config = server->thunkDatabase();
        assert(config != nullptr);
        const auto thunkName = normalizeThunkName(modulePath);

        std::string next;
        if (const auto *forward = config->forwardThunk(thunkName);
            forward && forward->hostLibrary && *forward->hostLibrary) {
            next = forward->hostLibrary;
        } else if (const char *baked = m_staticThunkContext->nextLibraryPath; baked && *baked) {
            next = baked;
        } else {
            next = utils::nextLibraryByName(modulePath, /*hostThunk=*/true);
        }
        // Expand ${...} (e.g. ARCH) on the host side, where those variables are known, then resolve the
        // path against this thunk's directory.
        std::string hostLib =
            utils::resolveNextLibrary(str::varexp(next, server->thunkVars()), modulePath);

        /// STEP: load host library
        m_hostLibraryHandle = dlopen(hostLib.c_str(), RTLD_NOW);
        if (!m_hostLibraryHandle) {
            const char *err = dlerror();
            log::logger().loreCriticalF("%s: failed to load host library %s (%s)", modulePath,
                         hostLib.c_str(), err ? err : "unknown error");
            std::abort();
        }

        /// STEP: resolve host library symbols
        // Resolve host-side real functions used by ProcFn<GuestToHost, Exec>.
        for (size_t i = 0; i < m_staticThunkContext->thisProcs.size; ++i) {
            auto &entry = m_staticThunkContext->thisProcs.arr[i];
            assert(entry.key != nullptr);
            entry.addr = dlsym(m_hostLibraryHandle, entry.key);
            if (!entry.addr) {
                const char *err = dlerror();
                log::logger().loreCritical(
                    "%1: failed to resolve symbol %2 from host library (%3)", modulePath,
                    entry.key, err ? err : "unknown error");
                std::abort();
            }
        }
    }

}

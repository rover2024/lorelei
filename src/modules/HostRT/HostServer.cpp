// SPDX-License-Identifier: MIT

#include "HostServer.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <dlfcn.h>
#include <limits.h>

#ifdef __linux__
#  include <link.h>
#endif

#include <lorelei/Support/Logging.h>
#include <lorelei/Support/StringExtras.h>
#include <lorelei/DLCall/Tools/VariadicAdaptor.h>

#include <Invocation.h>

namespace lore::mod {

    namespace {

        const char *pathGetName(const char *path) {
            const char *slashPos = std::strrchr(path, '/');
            if (!slashPos) {
                return path;
            }
            return slashPos + 1;
        }

        void getLibraryName(char *buffer, const char *path) {
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
            std::memcpy(buffer, start, end - start);
            buffer[end - start] = '\0';
        }

        // Resolve the module path of a library handle or a function address.
        void getModulePath(void *opaque, bool isHandle, char **ret) {
            if (isHandle) {
#ifdef __linux__
                struct link_map *lm = nullptr;
                if (dlinfo(opaque, RTLD_DI_LINKMAP, &lm) == 0 && lm && lm->l_name &&
                    lm->l_name[0] != '\0') {
                    *ret = lm->l_name;
                } else {
                    *ret = nullptr;
                }
#else
                *ret = nullptr;
#endif
            } else {
                Dl_info info;
                if (dladdr(opaque, &info) == 0) {
                    *ret = nullptr;
                } else {
                    *ret = const_cast<char *>(info.dli_fname);
                }
            }
        }

        // Look up forward/reversed thunk info for a (host) library path.
        void getThunkInfo(const char *path, bool isReverse, CThunkInfo *ret) {
            *ret = {};

            const auto *server = HostServer::instance();
            assert(server != nullptr);
            const auto *config = server->thunkDatabase();
            assert(config != nullptr);

            char nameBuf[PATH_MAX];
            getLibraryName(nameBuf, path ? path : "");

            std::string name(nameBuf);
            if (str::ends_with(name, "_HTL")) {
                name = name.substr(0, name.size() - 4);
            }

            if (isReverse) {
                ret->reversed = config->reversedThunk(name);
            } else {
                ret->forward = config->forwardThunk(name);
            }
        }

    }

    void *HostServer::emuAddr = nullptr;

    HostServer *HostServer::self = nullptr;

    HostServer::HostServer() {
        self = this;
    }

    HostServer::~HostServer() {
        self = nullptr;
    }

    void HostServer::setThunkDatabase(std::unique_ptr<ThunkDatabase> db) {
        assert(db != nullptr);
        m_thunkDatabase = std::move(db);
    }

    bool HostServer::isHostAddressNaive(void *addr) {
        // dladdr resolves addresses backed by a loaded module; guest addresses are not, so a
        // failed lookup (return 0) means the address is host-side.
        Dl_info info;
        return dladdr(addr, &info) == 0;
    }

    void HostServer::reenter(ReentryArguments *ra) {
        utils::Invocation::reenter(ra);
    }

}

namespace lore::utils {

    // Call the host function described by `ia` according to its calling convention, unpacking the
    // boxed arguments/return slot. Returns 0 on success, -1 for an unknown convention.
    int Invocation::invokeByConv(const InvocationArguments *ia) {
        // The coroutine layer hands us the opaque Invocation::InvocationArguments base; downcast to
        // the concrete protocol layout that carries the per-convention argument boxes.
        const auto ia1 = static_cast<const lore::InvocationArguments *>(ia);
        switch (ia1->conv) {
            case CC_Standard: {
                using Func = void (*)(void ** /*args*/, void * /*ret*/, void * /*metadata*/);

                auto func = reinterpret_cast<Func>(ia1->standard.proc);
                func(ia1->standard.args, ia1->standard.ret, ia1->standard.metadata);
                return 0;
            }

            case CC_StandardCallback: {
                using Func = void (*)(void * /*callback*/, void ** /*args*/, void * /*ret*/,
                                      void * /*metadata*/);

                auto func = reinterpret_cast<Func>(ia1->standardCallback.proc);
                func(ia1->standardCallback.callback, ia1->standardCallback.args,
                     ia1->standardCallback.ret, ia1->standardCallback.metadata);
                return 0;
            }

            case CC_Format: {
                VariadicAdaptor::callFormatBox64(ia1->format.proc, ia1->format.format,
                                                 ia1->format.args, ia1->format.ret);
                return 0;
            }

            case CC_ThreadEntry: {
                using Func = void *(*) (void *);

                auto func = reinterpret_cast<Func>(ia1->threadEntry.proc);
                *ia1->threadEntry.ret = func(ia1->threadEntry.arg);
                return 0;
            }

            default:
                return -1;
        }
    }

}

/// The host runtime's common entry, exported for the guest to resolve by name (see
/// \c GuestClient::setCommonHostEntry). Every \c DR_InvokeProc request the guest issues is routed
/// here by the dlcall plugin, which calls \c LoreCommonHostEntry(secondaryId, payload):
/// \a secondaryId is a \c lore::DLCallSecondaryID selecting the operation, and \a payload is that
/// operation's argument block. Each block layout below mirrors how \c lore::mod::GuestClient packs
/// it; results are written back through the out-pointers it contains.
extern "C" LOREHOSTRT_EXPORT void LoreCommonHostEntry(void *secondaryId, void *payload) {
    using namespace lore;
    using namespace lore::mod;
    using namespace lore::utils;

    const auto id = static_cast<DLCallSecondaryID>(reinterpret_cast<uintptr_t>(secondaryId));
    switch (id) {
        // payload: { const InvocationArguments *ia, ReentryArguments **outRa, int *outRet }.
        // outRet receives 1 if the host needs a guest reentry before finishing (outRa then points
        // at it), or 0 once the invocation is complete.
        case DS_InvokeFunction: {
            auto a = reinterpret_cast<void **>(payload);
            auto ia = reinterpret_cast<const InvocationArguments *>(a[0]);
            auto ra_ptr = reinterpret_cast<ReentryArguments **>(a[1]);
            auto ret = reinterpret_cast<int *>(a[2]);
            assert(ia && ra_ptr && ret);
            *ret = static_cast<int>(Invocation::invoke(ia, reinterpret_cast<void **>(ra_ptr)));
            break;
        }

        // payload: int *outRet -- resume the invocation the guest just serviced a reentry for, and
        // report its new status (1 = another reentry pending, 0 = complete), like DS_InvokeFunction.
        case DS_ResumeFunction: {
            auto ret = reinterpret_cast<int *>(payload);
            assert(ret);
            *ret = static_cast<int>(Invocation::resume());
            break;
        }

        // payload: { int level, const LogContext *context, const char *msg }.
        case DS_LogMessage: {
            auto a = reinterpret_cast<void **>(payload);
            assert(a);
            const auto level = static_cast<int>(reinterpret_cast<uintptr_t>(a[0]));
            const auto context = reinterpret_cast<const LogContext *>(a[1]);
            const auto msg = reinterpret_cast<const char *>(a[2]);
            assert(context && msg);
            Logger(*context).print(level, msg);
            break;
        }

        // payload: { void *opaque, bool isHandle, char **outPath }.
        case DS_GetModulePath: {
            auto a = reinterpret_cast<void **>(payload);
            assert(a);
            const bool isHandle = static_cast<bool>(reinterpret_cast<uintptr_t>(a[1]));
            getModulePath(a[0], isHandle, reinterpret_cast<char **>(a[2]));
            break;
        }

        // payload: { const char *path, bool isReverse, CThunkInfo *outInfo }.
        case DS_GetThunkInfo: {
            auto a = reinterpret_cast<void **>(payload);
            assert(a);
            const auto path = reinterpret_cast<const char *>(a[0]);
            const bool isReverse = static_cast<bool>(reinterpret_cast<uintptr_t>(a[1]));
            getThunkInfo(path, isReverse, reinterpret_cast<CThunkInfo *>(a[2]));
            break;
        }

        default:
            break;
    }
}

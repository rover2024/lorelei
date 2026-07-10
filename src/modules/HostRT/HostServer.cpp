// SPDX-License-Identifier: MIT

#include "HostServer.h"

#include <cstdio>
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

#include "LogCategory.h"

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

        // Derive the thunk base name from a library path or bare name: the basename with the rightmost
        // ".so" (and a trailing "_HTL") stripped.
        std::string thunkNameOf(const char *path) {
            char nameBuf[PATH_MAX];
            getLibraryName(nameBuf, path ? path : "");
            std::string name(nameBuf);
            if (str::ends_with(name, "_HTL")) {
                name = name.substr(0, name.size() - 4);
            }
            return name;
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

    void HostServer::configureThunkDiscovery(const std::map<std::string, std::string> &vars,
                                             const std::filesystem::path &overridePath,
                                             const std::string &hostArch, bool autoDiscover) {
        m_thunkVars = vars;
        m_hostArch = hostArch;
        m_thunkAutoDiscover = autoDiscover;

        // The initial database carries only the explicit override JSON, if one is configured. Packs are
        // layered under it later, as their thunks appear, via resolveForwardThunk(). An unset override
        // leaves the database empty. A set-but-unreadable one is worth a warning.
        auto db = std::make_unique<ThunkDatabase>();
        if (!overridePath.empty() && !db->load(overridePath, vars)) {
            log::logger().loreWarning("failed to load the thunk override database");
        }
        m_thunkDatabase = std::move(db);
    }

    void HostServer::getThunkInfo(const char *path, bool isReverse, CThunkInfo *ret) {
        *ret = {};

        std::lock_guard<std::mutex> lock(m_thunkMutex);
        assert(m_thunkDatabase != nullptr);

        const std::string name = thunkNameOf(path);
        if (isReverse) {
            // A reversed mapping is declared in a pack's JSON, loaded by an earlier forward lookup.
            ret->reversed = m_thunkDatabase->reversedThunk(name);
        } else {
            ret->forward = resolveForwardThunk(path, name);
        }
    }

    const CForwardThunkInfo *HostServer::resolveForwardThunk(const char *guestThunkPath,
                                                             const std::string &name) {
        // An explicit database entry (from a JSON, or an earlier convention hit) always wins.
        if (const auto *entry = m_thunkDatabase->forwardThunk(name)) {
            return entry;
        }
        if (!m_thunkAutoDiscover || !guestThunkPath || !*guestThunkPath) {
            return nullptr;
        }

        std::filesystem::path p(guestThunkPath);
        std::filesystem::path gtlDir = p.parent_path();

        // A guest thunk in the standard layout <prefix>/x86_64/lib/x86_64-LoreGTL/<name>.so names a
        // pack. Load that pack's JSON once (it may declare aliases, redirects or reversed thunks), then
        // fall back to the naming convention for the host thunk.
        if (gtlDir.filename() == "x86_64-LoreGTL") {
            std::filesystem::path prefix = gtlDir.parent_path().parent_path().parent_path();
            if (!prefix.empty()) {
                std::filesystem::path hostThunkDir = prefix / "lib" / (m_hostArch + "-LoreHTL");
                if (m_loadedPackPrefixes.insert(prefix.lexically_normal().string()).second) {
                    // A pack's ThunkDB.json is optional. Load it when present, and warn only if a
                    // present one fails to parse.
                    std::filesystem::path packJson = prefix / "share" / "lorelei" / "ThunkDB.json";
                    if (std::filesystem::exists(packJson) &&
                        !m_thunkDatabase->loadPack(packJson, gtlDir, hostThunkDir, m_thunkVars)) {
                        log::logger().loreWarning("failed to load a thunk pack database");
                    }
                    if (const auto *entry = m_thunkDatabase->forwardThunk(name)) {
                        return entry;
                    }
                }
                // Convention: <prefix>/lib/<arch>-LoreHTL/<name>_HTL.so. Materialize it only when the
                // host thunk is actually there.
                std::filesystem::path hostThunk = hostThunkDir / (name + "_HTL.so");
                if (std::filesystem::exists(hostThunk)) {
                    return m_thunkDatabase->materializeForward(name, p.string(), hostThunk.string(),
                                                               name + ".so");
                }
            }
            return nullptr;
        }

        // A bare name (no directory) comes from a reversed lookup's candidate list. Synthesize a
        // by-soname entry that the guest loader resolves through its own search path. The host thunk is
        // a placeholder here, since a reversed lookup loads only the guest side.
        if (!p.has_parent_path()) {
            return m_thunkDatabase->materializeForward(name, name + ".so", name + "_HTL.so",
                                                       name + ".so");
        }

        // A plain host library path (e.g. from convertHostProcAddress) resolves only against entries a
        // forward lookup already established.
        return nullptr;
    }

    bool HostServer::isHostAddressNaive(void *addr) {
        // dladdr resolves addresses backed by a host-loaded module. Guest objects are mapped by the
        // emulated loader and are not in the host link map, so a successful lookup means the address
        // is host-side (the mirror of the guest-side check, where dladdr sees the guest's objects).
        // ##FIXME: dladdr cannot see anonymous-mmap trampolines, so this naive classification is
        // sound only because callers unwrapTrampoline() first (itself an unchecked speculative
        // read at a fixed offset). A dedicated trampoline mmap arena would let both be range-checked
        // directly, the way separation mode uses emuAddr. Deferred.
        Dl_info info;
        return dladdr(addr, &info) != 0;
    }

    void HostServer::reenter(ReentryArguments *ra) {
        utils::Invocation::reenter(ra);
    }

}

namespace lore::utils {

    // Call the host function described by `ia` according to its calling convention, unpacking the
    // boxed arguments/return slot. Returns 0 on success, -1 for an unknown convention.
    int Invocation::invokeByConv(const InvocationArguments *ia) {
        // The coroutine layer hands us the opaque Invocation::InvocationArguments base. Downcast to
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
/// it. Results are written back through the out-pointers it contains.
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
            // A host function may have written to a host stdio stream. qemu tears the process down on
            // guest exit without running the host libc atexit handlers, so a fully-buffered stream
            // (output redirected or piped) would be lost. Flush at each completed invocation, a point
            // reached before exit.
            if (*ret == 0) {
                std::fflush(nullptr);
            }
            break;
        }

        // payload: int *outRet. Resume the invocation the guest just serviced a reentry for, and
        // report its new status (1 = another reentry pending, 0 = complete), like DS_InvokeFunction.
        case DS_ResumeFunction: {
            auto ret = reinterpret_cast<int *>(payload);
            assert(ret);
            *ret = static_cast<int>(Invocation::resume());
            if (*ret == 0) {
                std::fflush(nullptr);
            }
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
            HostServer::instance()->getThunkInfo(path, isReverse,
                                                 reinterpret_cast<CThunkInfo *>(a[2]));
            break;
        }

        default:
            break;
    }
}

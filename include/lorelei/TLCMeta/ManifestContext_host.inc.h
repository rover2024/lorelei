#include <cassert>

#include <lorelei/TLCMeta/ManifestConfig.h>
#include <lorelei/TLCMeta/ManifestlGlobal.h>
#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProc.h>

#include <lorelei/HostRT/HostThunkContext.h>
#include <lorelei/HostRT/HostServer.h>

#include <lorelei/Core/Connect/CallChecker.h>

#define LORETHUNK_HOST

#ifndef LORETHUNK_NO_KEYWORDS
#  define _TP      _HTP
#  define _TP_IMPL _HTP_IMPL
#endif

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_Builtin> {
        static constexpr bool isHost = true;
    };

}

#ifndef LORETHUNK_BUILD
namespace lorethunk {

    // Host Function
    // GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL
    template <auto F>
    struct MetaProc<F, CProcKind_HostFunction, CProcThunkPhase_HTP> {
        static constexpr CommonFunctionThunk invoke = nullptr;
    };

    template <auto F>
    struct MetaProc<F, CProcKind_HostFunction, CProcThunkPhase_HTP_IMPL> {
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct MetaProcExec<F, CProcKind_HostFunction> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    // Guest Function
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <auto F>
    struct MetaProc<F, CProcKind_GuestFunction, CProcThunkPhase_HTP> {
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct MetaProc<F, CProcKind_GuestFunction, CProcThunkPhase_HTP_IMPL> {
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct MetaProcExec<F, CProcKind_GuestFunction> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr CommonFunctionThunk invoke = nullptr;
    };

    // Host Callback
    // GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL
    template <class F>
    struct MetaProcCB<F, CProcKind_HostCallback, CProcThunkPhase_HTP> {
        static constexpr CommonCallbackThunk invoke = nullptr;
    };

    template <class F>
    struct MetaProcCB<F, CProcKind_HostCallback, CProcThunkPhase_HTP_IMPL> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct MetaProcCBExec<F, CProcKind_HostCallback> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    // Guest Callback
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <class F>
    struct MetaProcCB<F, CProcKind_GuestCallback, CProcThunkPhase_HTP> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct MetaProcCB<F, CProcKind_GuestCallback, CProcThunkPhase_HTP_IMPL> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct MetaProcCBExec<F, CProcKind_GuestCallback> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr CommonCallbackThunk invoke = nullptr;
    };

}

namespace lorethunk::proc {

    static inline lore::HostServer *server;

    struct LocalThunkContext {
        lore::HostThunkContext commonContext;
        lore::MemoryRegionCallChecker callChecker;

        LocalThunkContext() : commonContext(nullptr, nullptr) {
        }
    };

    static LocalThunkContext LTC;

}
#endif

#ifdef LORETHUNK_BUILD
#  include "private/ManifestContext_shared.inc.h"

namespace lorethunk::proc {

    static inline lore::HostServer *server;

    struct LocalThunkContext {
        lore::HostThunkContext commonContext;
        lore::MemoryRegionCallChecker callChecker;

        LocalThunkContext() : commonContext(LORETHUNK_MODULE_NAME, &staticProcInfoContext) {
            server = lore::HostServer::instance();
            assert(server != nullptr);

            preInitialize();
            commonContext.initialize();
            postInitialize();
        }

        void preInitialize();
        void postInitialize();
    };

    static LocalThunkContext LTC;
}

namespace lorethunk {

    // Host Function
    // GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL
    template <auto F>
    struct MetaProcExec<F, CProcKind_HostFunction> {
#  ifdef LORETHUNK_DIRECT_INVOKE
        static inline void *get() {
            return (void *) F;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return F(args...);
        }
#  else
        static inline void *get() {
            return proc::libraryFunctions[proc::getHostFunctionIndex<F>()].addr;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            // auto f = fopen("/tmp/1.txt", "a");
            // fprintf(f, "HostFunction: %s\n", proc::hostFunctions_HTPs[proc::getHostFunctionIndex<F>()].name);
            // fclose(f);
            return ((lore::remove_attr_t<F>) get())(args...);
        }
#  endif
    };

    // Guest Function
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <auto F>
    struct MetaProcExec<F, CProcKind_GuestFunction> {
        static inline void *get() {
            return proc::guestFunctions_GTPs[proc::getGuestFunctionIndex<F>()].addr;
        }
        static inline void invoke(void **args, void *ret, void *metadata) {
            (void) proc::server->runTaskFunction(get(), args, ret, metadata);
        }
    };

    // Host Callback
    // GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL
    template <class F>
    struct MetaProcCBExec<F, CProcKind_HostCallback> {
        template <class... Args>
        static inline auto invoke(void *callback, Args &&...args) {
            // auto f = fopen("/tmp/1.txt", "a");
            // fprintf(f, "HostCallback: %s\n", proc::hostCallbacks_HTPs[proc::getCallbackIndex<F>()].name);
            // fclose(f);
            return ((F) callback)(args...);
        }
    };

    // Guest Callback
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <class F>
    struct MetaProcCBExec<F, CProcKind_GuestCallback> {
        static inline void *get() {
            return proc::guestCallbacks_GTPs[proc::getCallbackIndex<F>()].addr;
        }
        static inline void invoke(void *callback, void **args, void *ret, void *metadata) {
            (void) proc::server->runTaskCallback(get(), callback, args, ret, metadata);
        }
    };

}
#endif
#include <cassert>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProc.h>
#include <lorelei/TLCMeta/ManifestlGlobal.h>

#include <lorelei/GuestRT/GuestThunkContext.h>
#include <lorelei/GuestRT/GuestClient.h>

#include <lorelei/Core/Connect/CallChecker.h>

#define LORETHUNK_GUEST

#ifndef LORETHUNK_NO_KEYWORDS
#  define _TP      _GTP
#  define _TP_IMPL _GTP_IMPL
#endif

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_Builtin> {
        static constexpr bool isHost = false;
    };

}

#ifndef LORETHUNK_BUILD
namespace lorethunk {

    // Host Function
    // GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL
    template <auto F>
    struct MetaProc<F, CProcKind_HostFunction, CProcThunkPhase_GTP> {
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct MetaProc<F, CProcKind_HostFunction, CProcThunkPhase_GTP_IMPL> {
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct MetaProcExec<F, CProcKind_HostFunction> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr CommonFunctionThunk invoke = nullptr;
    };

    // Guest Function
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <auto F>
    struct MetaProc<F, CProcKind_GuestFunction, CProcThunkPhase_GTP> {
        static constexpr CommonFunctionThunk invoke = nullptr;
    };

    template <auto F>
    struct MetaProc<F, CProcKind_GuestFunction, CProcThunkPhase_GTP_IMPL> {
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct MetaProcExec<F, CProcKind_GuestFunction> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr lore::remove_attr_t<F> invoke = nullptr;
    };

    // Host Callback
    // GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL
    template <class F>
    struct MetaProcCB<F, CProcKind_HostCallback, CProcThunkPhase_GTP> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct MetaProcCB<F, CProcKind_HostCallback, CProcThunkPhase_GTP_IMPL> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct MetaProcCBExec<F, CProcKind_HostCallback> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr CommonCallbackThunk invoke = nullptr;
    };

    // Guest Callback
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <class F>
    struct MetaProcCB<F, CProcKind_GuestCallback, CProcThunkPhase_GTP> {
        static constexpr CommonCallbackThunk invoke = nullptr;
    };

    template <class F>
    struct MetaProcCB<F, CProcKind_GuestCallback, CProcThunkPhase_GTP_IMPL> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct MetaProcCBExec<F, CProcKind_GuestCallback> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

}

namespace lorethunk::proc {

    static inline lore::GuestClient *client;

    struct LocalThunkContext {
        lore::GuestThunkContext commonContext;
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

    static inline lore::GuestClient *client;

    struct LocalThunkContext {
        lore::GuestThunkContext commonContext;
        lore::MemoryRegionCallChecker callChecker;

        LocalThunkContext() : commonContext(LORETHUNK_MODULE_NAME, &staticProcInfoContext) {
            client = lore::GuestClient::instance();
            assert(client != nullptr);

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
        static inline void *get() {
            return proc::hostFunctions_HTPs[proc::getHostFunctionIndex<F>()].addr;
        }
        static inline void invoke(void **args, void *ret, void *metadata) {
            (void) proc::client->invokeStandard(get(), args, ret, metadata);
        }
    };

    // Guest Function
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <auto F>
    struct MetaProcExec<F, CProcKind_GuestFunction> {
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
            return proc::libraryFunctions[proc::getGuestFunctionIndex<F>()].addr;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return ((lore::remove_attr_t<F>) get())(args...);
        }
#  endif
    };

    // Host Callback
    // GTP -> GTP_IMPL -> GRT -> EMU -> HRT -> HTP -> HTP_IMPL
    template <class F>
    struct MetaProcCBExec<F, CProcKind_HostCallback> {
        static inline void *get() {
            return proc::hostCallbacks_HTPs[proc::getCallbackIndex<F>()].addr;
        }
        static inline void invoke(void *callback, void **args, void *ret, void *metadata) {
            (void) proc::client->invokeStandardCallback(get(), callback, args, ret, metadata);
        }
    };

    // Guest Callback
    // HTP -> HTP_IMPL -> HRT -> EMU -> GRT -> GTP -> GTP_IMPL
    template <class F>
    struct MetaProcCBExec<F, CProcKind_GuestCallback> {
        template <class... Args>
        static inline auto invoke(void *callback, Args &&...args) {
            return ((F) callback)(args...);
        }
    };

}
#endif
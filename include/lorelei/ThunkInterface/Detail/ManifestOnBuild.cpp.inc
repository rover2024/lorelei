#include <lorelei/Support/STLTraitExtras.h>
#ifdef LORE_THUNK_HOST
#  include <lorelei/Modules/HostRT/HostThunkContext.h>
#  include <lorelei/Modules/HostRT/HostServer.h>
#else
#  include <lorelei/Modules/GuestRT/GuestThunkContext.h>
#  include <lorelei/Modules/GuestRT/GuestClient.h>
#endif
#include <lorelei/ThunkInterface/Detail/Traits.h>
#include <lorelei/ThunkInterface/Proc.h>

#include "ProcTable.cpp.inc"

namespace lore::thunk::detail {

    struct LocalThunkContext {
#ifdef LORE_THUNK_HOST
        mod::HostThunkContext commonContext;
#else
        mod::GuestThunkContext commonContext;
#endif

        LocalThunkContext() : commonContext(&staticThunkContext) {
            preInitialize();
            commonContext.initialize();
            postInitialize();
        }

        // Implemented in "ProcInit.cpp.inc"
        void preInitialize();
        void postInitialize();
    };

    static LocalThunkContext localContext;
}

namespace lore::thunk {

    /// Guest calls Host Function
    /// G_Entry -> G_Caller -> GRT -> EMU -> HRT -> H_Entry -> H_Caller
    template <auto F>
    struct ProcFn<F, GuestToHost, Exec> {
#ifdef LORE_THUNK_HOST
        // Host owns the real library function: call it directly.
#  ifdef LORE_THUNK_CONFIG_DIRECT_INVOKE
        static inline void *get() {
            return reinterpret_cast<void *>(F);
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return F(args...);
        }
#  else
        static inline void *get() {
            return detail::libraryFunctions[detail::getHostFunctionIndex<F>()].addr;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return (reinterpret_cast<remove_attr_t<F>>(get()))(args...);
        }
#  endif
#else
        // Guest crosses to host.
        static inline void *get() {
            return detail::hostFunctions_hostEntries[detail::getHostFunctionIndex<F>()].addr;
        }
        static inline void invoke(void **args, void *ret, void *metadata) {
            (void) mod::GuestClient::invokeStandard(get(), args, ret, metadata);
        }
#endif
    };

    /// Host calls Guest Function
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <auto F>
    struct ProcFn<F, HostToGuest, Exec> {
#ifdef LORE_THUNK_HOST
        // Host crosses to guest.
        static inline void *get() {
            return detail::guestFunctions_guestEntries[detail::getGuestFunctionIndex<F>()].addr;
        }
        static inline void invoke(void **args, void *ret, void *metadata) {
            mod::HostServer::reenterStandard(get(), args, ret, metadata);
        }
#else
        // Guest owns the real library function: call it directly.
#  ifdef LORE_THUNK_CONFIG_DIRECT_INVOKE
        static inline void *get() {
            return (void *) F;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return F(args...);
        }
#  else
        static inline void *get() {
            return detail::libraryFunctions[detail::getGuestFunctionIndex<F>()].addr;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return (reinterpret_cast<remove_attr_t<F>>(get()))(args...);
        }
#  endif
#endif
    };

    /// Guest calls Host Callback
    /// G_Entry -> G_Caller -> GRT -> EMU -> HRT -> H_Entry -> H_Caller
    template <class F>
    struct ProcCb<F, GuestToHost, Exec> {
#ifdef LORE_THUNK_HOST
        // Host owns the real callback: call it directly.
        template <class... Args>
        static inline auto invoke(void *callback, Args &&...args) {
            return (reinterpret_cast<F>(callback))(args...);
        }
#else
        // Guest crosses to host.
        static inline void *get() {
            return detail::hostCallbacks_hostEntries[detail::getCallbackIndex<F>()].addr;
        }
        static inline void invoke(void *callback, void **args, void *ret, void *metadata) {
            (void) mod::GuestClient::invokeStandardCallback(get(), callback, args, ret, metadata);
        }
#endif
    };

    /// Host calls Guest Callback
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <class F>
    struct ProcCb<F, HostToGuest, Exec> {
#ifdef LORE_THUNK_HOST
        // Host crosses to guest.
        static inline void *get() {
            return detail::guestCallbacks_guestEntries[detail::getCallbackIndex<F>()].addr;
        }
        static inline void invoke(void *callback, void **args, void *ret, void *metadata) {
            mod::HostServer::reenterStandardCallback(get(), callback, args, ret, metadata);
        }
#else
        // Guest owns the real callback: call it directly.
        template <class... Args>
        static inline auto invoke(void *callback, Args &&...args) {
            return (reinterpret_cast<F>(callback))(args...);
        }
#endif
    };

}

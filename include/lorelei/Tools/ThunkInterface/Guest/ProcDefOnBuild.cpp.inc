#include <lorelei/Base/Support/STLTraitExtras.h>
#include <lorelei/Modules/GuestRT/GuestThunkContext.h>
#include <lorelei/Modules/GuestRT/GuestSyscallClient.h>
#include <lorelei/Tools/ThunkInterface/Traits.h>
#include <lorelei/Tools/ThunkInterface/Proc.h>

#include "ProcDefShared.cpp.inc"

namespace lore::thunk::detail {

    struct LocalThunkContext {
        mod::GuestThunkContext commonContext;

        LocalThunkContext() : commonContext(&staticThunkContext) {
            preInitialize();
            commonContext.initialize();
            postInitialize();
        }

        // Implemented in "ProcImpl.cpp.inc"
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
        static inline void *get() {
            return detail::hostFunctions_hostEntries[detail::getHostFunctionIndex<F>()].addr;
        }
        static inline void invoke(void **args, void *ret, void *metadata) {
            (void) mod::GuestSyscallClient::invokeStandard(get(), args, ret, metadata);
        }
    };

    /// Host calls Guest Function
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <auto F>
    struct ProcFn<F, HostToGuest, Exec> {
#ifdef LORE_THUNK_CONFIG_DIRECT_INVOKE
        static inline void *get() {
            return (void *) F;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return F(args...);
        }
#else
        static inline void *get() {
            return detail::libraryFunctions[detail::getGuestFunctionIndex<F>()].addr;
        }
        template <typename... Args>
        static inline auto invoke(Args &&...args) {
            return (reinterpret_cast<remove_attr_t<F>>(get()))(args...);
        }
#endif
    };

    /// Guest calls Host Callback
    /// G_Entry -> G_Caller -> GRT -> EMU -> HRT -> H_Entry -> H_Caller
    template <class F>
    struct ProcCb<F, GuestToHost, Exec> {
        static inline void *get() {
            return detail::hostCallbacks_hostEntries[detail::getCallbackIndex<F>()].addr;
        }
        static inline void invoke(void *callback, void **args, void *ret, void *metadata) {
            (void) mod::GuestSyscallClient::invokeStandardCallback(get(), callback, args, ret,
                                                                   metadata);
        }
    };

    /// Host calls Guest Callback
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <class F>
    struct ProcCb<F, HostToGuest, Exec> {
        template <class... Args>
        static inline auto invoke(void *callback, Args &&...args) {
            return (reinterpret_cast<F>(callback))(args...);
        }
    };

}

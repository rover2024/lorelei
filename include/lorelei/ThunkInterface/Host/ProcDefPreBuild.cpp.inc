#include <lorelei/Base/Support/STLTraitExtras.h>
#include <lorelei/Modules/HostRT/HostThunkContext.h>
#include <lorelei/Tools/ThunkInterface/Traits.h>
#include <lorelei/Tools/ThunkInterface/Proc.h>

namespace lore::thunk::detail {

    struct LocalThunkContext {
        mod::HostThunkContext commonContext;

        LocalThunkContext() : commonContext(nullptr) {
        }
    };

    static LocalThunkContext localContext;

}

namespace lore::thunk {

    /// Guest calls Host Function
    /// G_Entry -> G_Caller -> GRT -> EMU -> HRT -> H_Entry -> H_Caller
    template <auto F>
    struct ProcFn<F, GuestToHost, Entry> {
        static constexpr CommonFunctionThunk invoke = nullptr;
    };

    template <auto F>
    struct ProcFn<F, GuestToHost, Caller> {
        static constexpr remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct ProcFn<F, GuestToHost, Exec> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr remove_attr_t<F> invoke = nullptr;
    };


    /// Host calls Guest Function
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <auto F>
    struct ProcFn<F, HostToGuest, Entry> {
        static constexpr remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct ProcFn<F, HostToGuest, Caller> {
        static constexpr remove_attr_t<F> invoke = nullptr;
    };

    template <auto F>
    struct ProcFn<F, HostToGuest, Exec> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr CommonFunctionThunk invoke = nullptr;
    };


    /// Guest calls Host Callback
    /// G_Entry -> G_Caller -> GRT -> EMU -> HRT -> H_Entry -> H_Caller
    template <class F>
    struct ProcCb<F, GuestToHost, Entry> {
        static constexpr CommonCallbackThunk invoke = nullptr;
    };

    template <class F>
    struct ProcCb<F, GuestToHost, Caller> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct ProcCb<F, GuestToHost, Exec> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };


    /// Host calls Guest Callback
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <class F>
    struct ProcCb<F, HostToGuest, Entry> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct ProcCb<F, HostToGuest, Caller> {
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
    };

    template <class F>
    struct ProcCb<F, HostToGuest, Exec> {
        static inline void *get() {
            return nullptr;
        }
        static constexpr CommonCallbackThunk invoke = nullptr;
    };

}


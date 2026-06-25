#include <lorelei/Support/STLTraitExtras.h>
#ifdef LORE_THUNK_HOST
#  include <lorelei/Modules/HostRT/HostThunkContext.h>
#else
#  include <lorelei/Modules/GuestRT/GuestThunkContext.h>
#endif
#include <lorelei/ThunkInterface/Detail/Traits.h>
#include <lorelei/ThunkInterface/Proc.h>

namespace lore::thunk::detail {

    struct LocalThunkContext {
#ifdef LORE_THUNK_HOST
        mod::HostThunkContext commonContext;
#else
        mod::GuestThunkContext commonContext;
#endif

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
#ifdef LORE_THUNK_HOST
        static constexpr CommonFunctionThunk invoke = nullptr;
#else
        static constexpr remove_attr_t<F> invoke = nullptr;
#endif
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
#ifdef LORE_THUNK_HOST
        static constexpr remove_attr_t<F> invoke = nullptr;
#else
        static constexpr CommonFunctionThunk invoke = nullptr;
#endif
    };


    /// Host calls Guest Function
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <auto F>
    struct ProcFn<F, HostToGuest, Entry> {
#ifdef LORE_THUNK_HOST
        static constexpr remove_attr_t<F> invoke = nullptr;
#else
        static constexpr CommonFunctionThunk invoke = nullptr;
#endif
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
#ifdef LORE_THUNK_HOST
        static constexpr CommonFunctionThunk invoke = nullptr;
#else
        static constexpr remove_attr_t<F> invoke = nullptr;
#endif
    };


    /// Guest calls Host Callback
    /// G_Entry -> G_Caller -> GRT -> EMU -> HRT -> H_Entry -> H_Caller
    template <class F>
    struct ProcCb<F, GuestToHost, Entry> {
#ifdef LORE_THUNK_HOST
        static constexpr CommonCallbackThunk invoke = nullptr;
#else
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
#endif
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
#ifdef LORE_THUNK_HOST
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
#else
        static constexpr CommonCallbackThunk invoke = nullptr;
#endif
    };


    /// Host calls Guest Callback
    /// H_Entry -> H_Caller -> HRT -> EMU -> GRT -> G_Entry -> G_Caller
    template <class F>
    struct ProcCb<F, HostToGuest, Entry> {
#ifdef LORE_THUNK_HOST
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
#else
        static constexpr CommonCallbackThunk invoke = nullptr;
#endif
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
#ifdef LORE_THUNK_HOST
        static constexpr CommonCallbackThunk invoke = nullptr;
#else
        static constexpr typename PrependCallbackToArgs<F>::type invoke = nullptr;
#endif
    };

}

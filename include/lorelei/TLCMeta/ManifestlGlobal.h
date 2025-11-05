#ifndef MANIFESTLGLOBAL_H
#define MANIFESTLGLOBAL_H

#define _USED __attribute__((used, visibility("hidden")))

#define _PROC _USED static

#define LORETHUNK_EXPORT __attribute__((visibility("default")))

#ifndef LORETHUNK_NO_KEYWORDS
#  define _GTP      CProcThunkPhase_GTP
#  define _GTP_IMPL CProcThunkPhase_GTP_IMPL
#  define _HTP      CProcThunkPhase_HTP
#  define _HTP_IMPL CProcThunkPhase_HTP_IMPL
#  define _GFN      CProcKind_GuestFunction
#  define _GCB      CProcKind_GuestCallback
#  define _HFN      CProcKind_HostFunction
#  define _HCB      CProcKind_HostCallback
#endif

namespace lorethunk {

    using CommonFunctionThunk = void (*)(void *[] /*args*/, void * /*ret*/, void * /*metadata*/);

    using CommonCallbackThunk = void (*)(void * /*callback*/, void *[] /*args*/, void * /*ret*/,
                                         void * /*metadata*/);

    template <class F>
    struct PrependCallbackToArgs;

    template <class Ret, class... Args>
    struct PrependCallbackToArgs<Ret (*)(Args...)> {
        using type = Ret (*)(void *, Args...);
    };

    template <class Ret, class... Args>
    struct PrependCallbackToArgs<Ret (*)(Args...) noexcept> {
        using type = Ret (*)(void *, Args...);
    };

    template <class Ret, class... Args>
    struct PrependCallbackToArgs<Ret (*)(Args..., ...)> {
        using type = Ret (*)(void *, Args..., ...);
    };

    template <class Ret, class... Args>
    struct PrependCallbackToArgs<Ret (*)(Args..., ...) noexcept> {
        using type = Ret (*)(void *, Args..., ...);
    };

}

#endif // MANIFESTLGLOBAL_H

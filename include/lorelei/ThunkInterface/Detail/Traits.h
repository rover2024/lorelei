// SPDX-License-Identifier: MIT

#ifndef LORE_THUNKINTERFACE_TRAITS_H
#define LORE_THUNKINTERFACE_TRAITS_H

namespace lore::thunk {

    /// CommonFunctionThunk - The uniform signature every generated function thunk presents: the
    /// typed arguments packed as an \c args[] buffer, a return slot, and per-proc metadata.
    using CommonFunctionThunk = void (*)(void *[] /*args*/, void * /*ret*/, void * /*metadata*/);

    /// CommonCallbackThunk - Like \c CommonFunctionThunk, with a leading pointer to the guest
    /// callback being invoked.
    using CommonCallbackThunk = void (*)(void * /*callback*/, void *[] /*args*/, void * /*ret*/,
                                         void * /*metadata*/);

    /// PrependCallbackToArgs - Rewrites a function-pointer type to take a leading \c void* (the
    /// callback context), preserving the return type, arguments, variadic tail and \c noexcept.
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

#endif // LORE_THUNKINTERFACE_TRAITS_H

#ifndef LORE_TOOLS_THUNKINTERFACE_TRAITS_H
#define LORE_TOOLS_THUNKINTERFACE_TRAITS_H

namespace lore::thunk {

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

#endif // LORE_TOOLS_THUNKINTERFACE_TRAITS_H

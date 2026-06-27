// SPDX-License-Identifier: MIT

#ifndef LORE_SUPPORT_STLTRAITEXTRAS_H
#define LORE_SUPPORT_STLTRAITEXTRAS_H

#include <tuple>

namespace lore {

    /// function_info - Extracts a function's return type and argument tuple from its type.
    ///
    /// Works for plain function types and function pointers, variadic or not; a trailing \c ... is
    /// dropped from \c tuple_type.
    template <class F>
    struct function_info;

    template <class Ret, class... Args>
    struct function_info<Ret (*)(Args...)> {
        using return_type = Ret;
        using tuple_type = std::tuple<Args...>;
    };

    template <class Ret, class... Args>
    struct function_info<Ret (*)(Args..., ...)> {
        using return_type = Ret;
        using tuple_type = std::tuple<Args...>;
    };

    template <class Ret, class... Args>
    struct function_info<Ret(Args...)> {
        using return_type = Ret;
        using tuple_type = std::tuple<Args...>;
    };

    template <class Ret, class... Args>
    struct function_info<Ret(Args..., ...)> {
        using return_type = Ret;
        using tuple_type = std::tuple<Args...>;
    };

    /// remove_attr - Yields the plain type of the function value \c F with any function attributes
    /// stripped (for example a format or nonnull attribute on a function pointer).
    template <auto F>
    struct remove_attr {
        static constexpr auto F_clean = F;
        using type = std::decay_t<decltype(F_clean)>;
    };

    template <auto F>
    using remove_attr_t = typename remove_attr<F>::type;

    template <class F>
    using return_type_of = typename function_info<F>::return_type;

}

#endif // LORE_SUPPORT_STLTRAITEXTRAS_H

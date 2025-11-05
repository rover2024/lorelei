#ifndef TRAITS_H
#define TRAITS_H

#include <tuple>

namespace lore {

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

#endif // TRAITS_H

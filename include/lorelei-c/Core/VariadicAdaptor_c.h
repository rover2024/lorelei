#ifndef LORECORE_VARIADICDUMPCDEFS_H
#define LORECORE_VARIADICDUMPCDEFS_H

#ifdef __cplusplus
#  include <type_traits>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum CVargType {
    CVargType_Void,
    CVargType_Char,
    CVargType_UChar,
    CVargType_Short,
    CVargType_UShort,
    CVargType_Int,
    CVargType_UInt,
    CVargType_Long,
    CVargType_ULong,
    CVargType_LongLong,
    CVargType_ULongLong,
    CVargType_Float,
    CVargType_Double,
    CVargType_Pointer,
};

struct CVargEntry {
    int type;
    union {
        char c;
        unsigned char uc;
        short s;
        unsigned short us;
        int i;
        unsigned int u;
        long l;
        unsigned ul;
        long long ll;
        unsigned long long ull;
        float f;
        double d;
        void *p;
    };
};

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

template <typename T, class = std::void_t<>>
struct _Enum2Underlying {
    using type = T;
};

template <typename T>
struct _Enum2Underlying<T, std::enable_if_t<std::is_enum_v<T>>> {
    using type = std::underlying_type_t<T>;
};

template <class T>
static inline constexpr CVargType CVargTypeID(T x) {
    using T1 = std::remove_cv_t<T>;
    using T2 = typename _Enum2Underlying<T1>::type;
    using V = T2;
    if constexpr (std::is_same_v<V, void>) {
        return CVargType_Void;
    } else if constexpr (std::is_same_v<V, char>) {
        return CVargType_Char;
    } else if constexpr (std::is_same_v<V, unsigned char>) {
        return CVargType_UChar;
    } else if constexpr (std::is_same_v<V, short>) {
        return CVargType_Short;
    } else if constexpr (std::is_same_v<V, unsigned short>) {
        return CVargType_UShort;
    } else if constexpr (std::is_same_v<V, int>) {
        return CVargType_Int;
    } else if constexpr (std::is_same_v<V, unsigned int>) {
        return CVargType_UInt;
    } else if constexpr (std::is_same_v<V, long>) {
        return CVargType_Long;
    } else if constexpr (std::is_same_v<V, unsigned long>) {
        return CVargType_ULong;
    } else if constexpr (std::is_same_v<V, long long>) {
        return CVargType_LongLong;
    } else if constexpr (std::is_same_v<V, unsigned long long>) {
        return CVargType_ULongLong;
    } else if constexpr (std::is_same_v<V, float>) {
        return CVargType_Float;
    } else if constexpr (std::is_same_v<V, double>) {
        return CVargType_Double;
    } else {
        return CVargType_Pointer;
    }
}

template <class T>
static inline constexpr auto *_CVargValueRef(CVargEntry &v) {
    using T1 = std::remove_cv_t<T>;
    using T2 = typename _Enum2Underlying<T1>::type;
    using V = T2;
    if constexpr (std::is_same_v<V, char>) {
        return &v.c;
    } else if constexpr (std::is_same_v<V, unsigned char>) {
        return &v.uc;
    } else if constexpr (std::is_same_v<V, short>) {
        return &v.s;
    } else if constexpr (std::is_same_v<V, unsigned short>) {
        return &v.us;
    } else if constexpr (std::is_same_v<V, int>) {
        return &v.i;
    } else if constexpr (std::is_same_v<V, unsigned int>) {
        return &v.u;
    } else if constexpr (std::is_same_v<V, long>) {
        return &v.l;
    } else if constexpr (std::is_same_v<V, unsigned long>) {
        return &v.ul;
    } else if constexpr (std::is_same_v<V, long long>) {
        return &v.ll;
    } else if constexpr (std::is_same_v<V, unsigned long long>) {
        return &v.ull;
    } else if constexpr (std::is_same_v<V, float>) {
        return &v.f;
    } else if constexpr (std::is_same_v<V, double>) {
        return &v.d;
    } else {
        return &v.p;
    }
}

#  define CVargValueRef(x, V) _CVargValueRef<__typeof__(x)>(V)

#  define CVargValue(x, V) (*CVargValueRef(x, V))

template <class T>
static inline CVargEntry CVargGet(T x) {
    CVargEntry result;
    result.type = CVargTypeID(x);
    if constexpr (std::is_pointer_v<T>) {
        result.p = (void *) x;
    } else {
        CVargValue(x, result) = static_cast<__typeof__(CVargValue(x, result))>(x);
    }
    return result;
}
#else
#  define CVargTypeID(x)                                                                           \
      ({                                                                                           \
        __attribute__((unused)) __typeof__(x) y;                                                   \
        __builtin_choose_expr(__builtin_classify_type(y) == 5, CVargType_Pointer,                  \
                              _Generic(y,                                                          \
                              char: CVargType_Char,                                                \
                              unsigned char: CVargType_UChar,                                      \
                              short: CVargType_Short,                                              \
                              unsigned short: CVargType_UShort,                                    \
                              int: CVargType_Int,                                                  \
                              unsigned int: CVargType_UInt,                                        \
                              long: CVargType_Long,                                                \
                              unsigned long: CVargType_ULong,                                      \
                              long long: CVargType_LongLong,                                       \
                              unsigned long long: CVargType_ULongLong,                             \
                              float: CVargType_Float,                                              \
                              double: CVargType_Double,                                            \
                              default: CVargType_Pointer));                                        \
      })

#  define CVargValueRef(x, V)                                                                      \
      ({                                                                                           \
        __attribute__((unused)) __typeof__(x) y;                                                   \
        __builtin_choose_expr(__builtin_classify_type(y) == 5, &(V).p,                             \
                              _Generic(y,                                                          \
                              char: &(V).c,                                                        \
                              unsigned char: &(V).uc,                                              \
                              short: &(V).s,                                                       \
                              unsigned short: &(V).us,                                             \
                              int: &(V).i,                                                         \
                              unsigned int: &(V).u,                                                \
                              long: &(V).l,                                                        \
                              unsigned long: &(V).ul,                                              \
                              long long: &(V).ll,                                                  \
                              unsigned long long: &(V).ull,                                        \
                              float: &(V).f,                                                       \
                              double: &(V).d,                                                      \
                              default: &(V).p));                                                   \
      })

#  define CVargValue(x, V) *CVargValueRef(x, V)

#  define CVargGet(x)                                                                              \
      ({                                                                                           \
        struct CVargEntry result = {.type = CVargTypeID(x)};                                       \
        __auto_type p = CVargValueRef((x), result);                                                \
        *p = (__typeof__(*p)) x;                                                                   \
        result;                                                                                    \
      })
#endif

#endif // LORECORE_VARIADICDUMPCDEFS_H

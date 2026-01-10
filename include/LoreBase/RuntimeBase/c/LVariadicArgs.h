#ifndef LORE_BASE_RUNTIMEBASE_LVARIADICARGS_H
#define LORE_BASE_RUNTIMEBASE_LVARIADICARGS_H

#ifdef __cplusplus
#  include <type_traits>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum LVargType {
    LVargType_Void,
    LVargType_Char,
    LVargType_UChar,
    LVargType_Short,
    LVargType_UShort,
    LVargType_Int,
    LVargType_UInt,
    LVargType_Long,
    LVargType_ULong,
    LVargType_LongLong,
    LVargType_ULongLong,
    LVargType_Float,
    LVargType_Double,
    LVargType_Pointer,
};

struct LVargEntry {
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

static inline int LVargEntryLength(struct LVargEntry *argv) {
    int argc = 0;
    for (struct LVargEntry *p = argv; p && p->type; ++p) {
        argc++;
    }
    return argc;
}

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
static inline constexpr LVargType _LVargTypeID() {
    using T1 = std::remove_cv_t<T>;
    using T2 = typename _Enum2Underlying<T1>::type;
    using V = T2;
    if constexpr (std::is_same_v<V, void>) {
        return LVargType_Void;
    } else if constexpr (std::is_same_v<V, char>) {
        return LVargType_Char;
    } else if constexpr (std::is_same_v<V, unsigned char>) {
        return LVargType_UChar;
    } else if constexpr (std::is_same_v<V, short>) {
        return LVargType_Short;
    } else if constexpr (std::is_same_v<V, unsigned short>) {
        return LVargType_UShort;
    } else if constexpr (std::is_same_v<V, int>) {
        return LVargType_Int;
    } else if constexpr (std::is_same_v<V, unsigned int>) {
        return LVargType_UInt;
    } else if constexpr (std::is_same_v<V, long>) {
        return LVargType_Long;
    } else if constexpr (std::is_same_v<V, unsigned long>) {
        return LVargType_ULong;
    } else if constexpr (std::is_same_v<V, long long>) {
        return LVargType_LongLong;
    } else if constexpr (std::is_same_v<V, unsigned long long>) {
        return LVargType_ULongLong;
    } else if constexpr (std::is_same_v<V, float>) {
        return LVargType_Float;
    } else if constexpr (std::is_same_v<V, double>) {
        return LVargType_Double;
    } else {
        return LVargType_Pointer;
    }
}

#  define LVargTypeID(x) _LVargTypeID<__typeof__(x)>()

template <class T>
static inline constexpr auto *_LVargValueRef(LVargEntry &v) {
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

#  define LVargValueRef(x, V) _LVargValueRef<__typeof__(x)>(V)

#  define LVargValue(x, V) (*LVargValueRef(x, V))

template <class T>
static inline LVargEntry LVargGet(T x) {
    LVargEntry result;
    result.type = _LVargTypeID<T>();
    if constexpr (std::is_pointer_v<T>) {
        result.p = (void *) x;
    } else {
        LVargValue(x, result) = static_cast<__typeof__(LVargValue(x, result))>(x);
    }
    return result;
}
#else
#  define LVargTypeID(x)                                                                           \
      ({                                                                                           \
        __attribute__((unused)) __typeof__(x) y;                                                   \
        __builtin_choose_expr(__builtin_classify_type(y) == 5, LVargType_Pointer,                  \
                              _Generic(y,                                                          \
                                  char: LVargType_Char,                                            \
                                  unsigned char: LVargType_UChar,                                  \
                                  short: LVargType_Short,                                          \
                                  unsigned short: LVargType_UShort,                                \
                                  int: LVargType_Int,                                              \
                                  unsigned int: LVargType_UInt,                                    \
                                  long: LVargType_Long,                                            \
                                  unsigned long: LVargType_ULong,                                  \
                                  long long: LVargType_LongLong,                                   \
                                  unsigned long long: LVargType_ULongLong,                         \
                                  float: LVargType_Float,                                          \
                                  double: LVargType_Double,                                        \
                                  default: LVargType_Pointer));                                    \
      })

#  define LVargValueRef(x, V)                                                                      \
      ({                                                                                           \
        __attribute__((unused)) __typeof__(x) y;                                                   \
        __builtin_choose_expr(__builtin_classify_type(y) == 5, &(V).p,                             \
                              _Generic(y,                                                          \
                                  char: &(V).c,                                                    \
                                  unsigned char: &(V).uc,                                          \
                                  short: &(V).s,                                                   \
                                  unsigned short: &(V).us,                                         \
                                  int: &(V).i,                                                     \
                                  unsigned int: &(V).u,                                            \
                                  long: &(V).l,                                                    \
                                  unsigned long: &(V).ul,                                          \
                                  long long: &(V).ll,                                              \
                                  unsigned long long: &(V).ull,                                    \
                                  float: &(V).f,                                                   \
                                  double: &(V).d,                                                  \
                                  default: &(V).p));                                               \
      })

#  define LVargValue(x, V) *LVargValueRef(x, V)

#  define LVargGet(x)                                                                              \
      ({                                                                                           \
        struct LVargEntry result = {.type = LVargTypeID(x)};                                       \
        __auto_type p = LVargValueRef((x), result);                                                \
        *p = (__typeof__(*p)) x;                                                                   \
        result;                                                                                    \
      })
#endif

#endif // LORE_BASE_RUNTIMEBASE_LVARIADICARGS_H

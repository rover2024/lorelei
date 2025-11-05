#ifndef LORECORE_VARIADICDUMPCDEFS_H
#define LORECORE_VARIADICDUMPCDEFS_H

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

#define CVargTypeID(x)                                                                             \
    ({                                                                                             \
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

#define CVargValueRef(x, V)                                                                        \
    ({                                                                                             \
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

#define CVargValue(x, V) *CVargValueRef(x, V)

#define CVargGet(x)                                                                                \
    ({                                                                                             \
        struct CVargEntry result = {.type = CVargTypeID(x)};                                       \
        __auto_type p = CVargValueRef((x), result);                                                \
        *p = (__typeof__(*p)) x;                                                                   \
        result;                                                                                    \
    })

#ifdef __cplusplus
}
#endif

#endif // LORECORE_VARIADICDUMPCDEFS_H

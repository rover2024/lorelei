#ifndef LORESHARED_H
#define LORESHARED_H

#include <stdarg.h>

#include <lorelei/lorelei_global.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LORE_THUNK_LIBRARY_DATA {
    const char *name;

    // paths
    const char *gtl;
    const char *htl;
    const char *hl;

    int dependencyCount;
    const char **dependencies;
};

struct LORE_HOST_LIBRARY_DATA {
    const char *name;
    const char *fileName;

    int thunksCount;
    const char **thunks;
};

//
// Path APIs
//
LORELEI_EXPORT int Lore_RevealLibraryPath(char *buffer, const void *addr, int followSymlink);

LORELEI_EXPORT void Lore_GetLibraryName(char *buffer, const char *path);


//
// Variadic argument APIs
//
enum LORE_VARG_TYPE {
    LORE_VT_VOID,
    LORE_VT_CHAR,
    LORE_VT_UCHAR,
    LORE_VT_SHORT,
    LORE_VT_USHORT,
    LORE_VT_INT,
    LORE_VT_UINT,
    LORE_VT_LONG,
    LORE_VT_ULONG,
    LORE_VT_LONGLONG,
    LORE_VT_ULONGLONG,
    LORE_VT_FLOAT,
    LORE_VT_DOUBLE,
    LORE_VT_POINTER,
};

struct LORE_VARG_ENTRY {
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

#define LORE_VARG_TYPE_ID(x)                                                                                           \
    ({                                                                                                                 \
        __attribute__((unused)) __typeof__(x) y;                                                                       \
        __builtin_choose_expr(__builtin_classify_type(y) == 5, LORE_VT_POINTER,                                        \
                              _Generic(y,                                                                              \
                              char: LORE_VT_CHAR,                                                                      \
                              unsigned char: LORE_VT_UCHAR,                                                            \
                              short: LORE_VT_SHORT,                                                                    \
                              unsigned short: LORE_VT_USHORT,                                                          \
                              int: LORE_VT_INT,                                                                        \
                              unsigned int: LORE_VT_UINT,                                                              \
                              long: LORE_VT_LONG,                                                                      \
                              unsigned long: LORE_VT_ULONG,                                                            \
                              long long: LORE_VT_LONGLONG,                                                             \
                              unsigned long long: LORE_VT_ULONGLONG,                                                   \
                              float: LORE_VT_FLOAT,                                                                    \
                              double: LORE_VT_DOUBLE,                                                                  \
                              default: LORE_VT_POINTER));                                                              \
    })

#define LORE_VARG_VALUE_REF(x, V)                                                                                      \
    ({                                                                                                                 \
        __attribute__((unused)) __typeof__(x) y;                                                                       \
        __builtin_choose_expr(__builtin_classify_type(y) == 5, &(V).p,                                                 \
                              _Generic(y,                                                                              \
                              char: &(V).c,                                                                            \
                              unsigned char: &(V).uc,                                                                  \
                              short: &(V).s,                                                                           \
                              unsigned short: &(V).us,                                                                 \
                              int: &(V).i,                                                                             \
                              unsigned int: &(V).u,                                                                    \
                              long: &(V).l,                                                                            \
                              unsigned long: &(V).ul,                                                                  \
                              long long: &(V).ll,                                                                      \
                              unsigned long long: &(V).ull,                                                            \
                              float: &(V).f,                                                                           \
                              double: &(V).d,                                                                          \
                              default: &(V).p));                                                                       \
    })

#define LORE_VARG_VALUE(x, V) *LORE_VARG_VALUE_REF(x, V)

#define LORE_VARG(x)                                                                                                   \
    ({                                                                                                                 \
        struct LORE_VARG_ENTRY result = {.type = LORE_VARG_TYPE_ID(x)};                                                \
        __auto_type p = LORE_VARG_VALUE_REF((x), result);                                                              \
        *p = (__typeof__(*p)) x;                                                                                       \
        result;                                                                                                        \
    })

LORELEI_EXPORT int Lore_ExtractPrintFArgs(const char *format, va_list ap, struct LORE_VARG_ENTRY *out);

LORELEI_EXPORT int Lore_ExtractSScanFArgs(const char *buffer, const char *format, va_list ap,
                                          struct LORE_VARG_ENTRY *out);

LORELEI_EXPORT int Lore_ExtractScanFArgs(const char *format, va_list ap, struct LORE_VARG_ENTRY *out);

LORELEI_EXPORT void Lore_VAFmtCall(void *func, int argc1, struct LORE_VARG_ENTRY *argv1, int argc2,
                                   struct LORE_VARG_ENTRY *argv2, struct LORE_VARG_ENTRY *ret);

LORELEI_EXPORT void Lore_VAFmtCallV(void *func, int argc1, struct LORE_VARG_ENTRY *argv1, int argc2,
                                    struct LORE_VARG_ENTRY *argv2, struct LORE_VARG_ENTRY *ret);

LORELEI_EXPORT void Lore_AVCall(void *func, void **args, void *ret, const char *convention);


//
// Callback set
//


#ifdef __cplusplus
}
#endif

#endif // LORESHARED_H
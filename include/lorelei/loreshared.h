#ifndef LORESHARED_H
#define LORESHARED_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

#include <lorelei/lorelei_global.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Library data structures
//
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

#define LORE_VARG_TYPE_ID(x)                                                                       \
    ({                                                                                             \
        __attribute__((unused)) __typeof__(x) y;                                                   \
        __builtin_choose_expr(__builtin_classify_type(y) == 5, LORE_VT_POINTER,                    \
                              _Generic(y,                                                          \
                              char: LORE_VT_CHAR,                                                  \
                              unsigned char: LORE_VT_UCHAR,                                        \
                              short: LORE_VT_SHORT,                                                \
                              unsigned short: LORE_VT_USHORT,                                      \
                              int: LORE_VT_INT,                                                    \
                              unsigned int: LORE_VT_UINT,                                          \
                              long: LORE_VT_LONG,                                                  \
                              unsigned long: LORE_VT_ULONG,                                        \
                              long long: LORE_VT_LONGLONG,                                         \
                              unsigned long long: LORE_VT_ULONGLONG,                               \
                              float: LORE_VT_FLOAT,                                                \
                              double: LORE_VT_DOUBLE,                                              \
                              default: LORE_VT_POINTER));                                          \
    })

#define LORE_VARG_VALUE_REF(x, V)                                                                  \
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

#define LORE_VARG_VALUE(x, V) *LORE_VARG_VALUE_REF(x, V)

#define LORE_VARG(x)                                                                               \
    ({                                                                                             \
        struct LORE_VARG_ENTRY result = {.type = LORE_VARG_TYPE_ID(x)};                            \
        __auto_type p = LORE_VARG_VALUE_REF((x), result);                                          \
        *p = (__typeof__(*p)) x;                                                                   \
        result;                                                                                    \
    })

LORELEI_EXPORT int Lore_ExtractPrintFArgs(const char *format, va_list ap,
                                          struct LORE_VARG_ENTRY *out);

LORELEI_EXPORT int Lore_ExtractSScanFArgs(const char *buffer, const char *format, va_list ap,
                                          struct LORE_VARG_ENTRY *out);

LORELEI_EXPORT int Lore_ExtractScanFArgs(const char *format, va_list ap,
                                         struct LORE_VARG_ENTRY *out);

LORELEI_EXPORT void Lore_VAFmtCall(void *func, int argc1, struct LORE_VARG_ENTRY *argv1, int argc2,
                                   struct LORE_VARG_ENTRY *argv2, struct LORE_VARG_ENTRY *ret);

LORELEI_EXPORT void Lore_VAFmtCallV(void *func, int argc1, struct LORE_VARG_ENTRY *argv1, int argc2,
                                    struct LORE_VARG_ENTRY *argv2, struct LORE_VARG_ENTRY *ret);

LORELEI_EXPORT void Lore_AVCall(void *func, void **args, void *ret, const char *convention);


//
// Callback trampoline APIs
//

/// \example: Callback trampoline example (x86_64)
/// \code
///     extern __thread void *Lore_HRTThreadCallback;
///
///     typedef int (*CMP)(const void *, const void *);
///
///     // Callback thunk for CMP
///     static int __thunk_CMP(const void *a, const void *b) {
///         void *callback = Lore_HRTThreadCallback;
///
///         // Return to guest code
///         // ...
///     }
///
///     // GCC-Arm64 doesn't support naked functions
///     __asm__(".text                                                  \n\t"
///             ".type __qsort_THUNK_ASM, @function                     \n\t"
///             "__qsort_THUNK_ASM:                                     \n\t"
///                 "movq    -8(%r10), %r10                             \n\t"
///                 "movq   Lore_HRTThreadCallback@gottpoff(%rip), %rax \n\t"
///                 "movq   %r10, %fs:(%rax)                            \n\t"
///                 "jmp    __thunk_CMP"
///     );
///
///
///     // Thunk entry getter for CMP
///     static inline void *__qsort_THUNK_ASM_get() {
///         void *addr;
///         __asm__("lea __qsort_thunk_entry(%%rip), %0" : "=r"(addr));
///         return addr;
///     }
///
///     // Trampoline allocator for CMP
///     static CMP __qsort_thunk_alloc(CMP cmp) {
///         static struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = NULL;
///         if (!trampoline) {
///             trampoline = Lore_AllocCallbackTrampoline(1, __qsort_THUNK_ASM_get());
///         }
///         struct LORE_CALLBACK_TRAMPOLINE *t = &trampoline->trampoline[0];
///         while (t->saved_callback) {
///             if (t->saved_callback == cmp) {
///                 return (CMP) t->thunk_instr;
///             }
///             t++;
///         }
///         t->saved_callback = cmp;
///         return (CMP) t->thunk_instr;
///     }
///
///     // Host thunk entry
///     void qsort(void *base, size_t nmemb, size_t size, CMP cmp) {
///         cmp = __qsort_thunk_alloc(cmp);
///
///         // Call host qsort
///         // ...
///     }
/// \endcode

struct LORE_CALLBACK_TRAMPOLINE {
    void *saved_callback;
    char thunk_instr[16]; // lea -7(%rip), %r10; jmp jump_instr
};

struct LORE_CALLBACK_TRAMPOLINE_CONTEXT {
    char jump_instr[16]; // mov target, %rax; jmp *%rax
    size_t count;        // length of the "trampoline"
    struct LORE_CALLBACK_TRAMPOLINE trampoline[];
};

LORELEI_EXPORT struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *Lore_AllocCallbackTrampoline(size_t count,
                                                                                     void *target);

LORELEI_EXPORT void Lore_FreeCallbackTrampoline(struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *ctx);

#ifdef __cplusplus
}
#endif

#endif // LORESHARED_H
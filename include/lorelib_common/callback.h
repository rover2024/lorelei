#ifndef LORELIB_COMMON_CALLBACK_H
#define LORELIB_COMMON_CALLBACK_H

#include <stddef.h>

#include <lorelei/loreshared.h>


//
// Last callback getter
//
#define LORELIB_GET_LAST_CALLBACK_X86_64(NAME)                                                     \
    void *NAME;                                                                                    \
    asm volatile("mov %%r11, %0" : "=r"(NAME)::"memory");

#define LORELIB_GET_LAST_CALLBACK_ARM64(NAME)                                                      \
    void *NAME;                                                                                    \
    asm volatile("mov %0, x16" : "=r"(NAME)::"memory");

#define LORELIB_GET_LAST_CALLBACK_RISCV64(NAME)                                                    \
    void *NAME;                                                                                    \
    asm volatile("mv %0, t1" : "=r"(NAME)::"memory");

#define LORELIB_GET_LAST_HCB LORELIB_GET_LAST_CALLBACK_X86_64

#ifdef __x86_64__
#  define LORELIB_GET_LAST_GCB LORELIB_GET_LAST_CALLBACK_X86_64
#elif defined(__aarch64__)
#  define LORELIB_GET_LAST_GCB LORELIB_GET_LAST_GCB_ARM64
#elif defined(__riscv)
#  define LORELIB_GET_LAST_GCB LORELIB_GET_LAST_GCB_RISCV64
#endif



//
// Thunk allocator
//
#define LORELIB_GCB_THUNK_ALLOCATOR(NAME, TARGET)                                                  \
    static __typeof__(&TARGET) NAME(void *input) {                                                 \
        static __thread struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = NULL;                \
        if (!trampoline) {                                                                         \
            trampoline = Lore_AllocCallbackTrampoline(1, TARGET);                                  \
        }                                                                                          \
        struct LORE_CALLBACK_TRAMPOLINE *t = &trampoline->trampoline[0];                           \
        while (t->saved_callback) {                                                                \
            if (t->saved_callback == input) {                                                      \
                return (void *) t->thunk_instr;                                                    \
            }                                                                                      \
            t++;                                                                                   \
        }                                                                                          \
        t->saved_callback = input;                                                                 \
        return (void *) t->thunk_instr;                                                            \
    }

#define LORELIB_HCB_THUNK_ALLOCATOR(NAME, TARGET)                                                  \
    static __typeof__(&TARGET) NAME(void *input) {                                                 \
        static __thread struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = NULL;                \
        if (!trampoline) {                                                                         \
            trampoline = Lore_AllocCallbackTrampoline(1, TARGET);                                  \
        }                                                                                          \
        struct LORE_CALLBACK_TRAMPOLINE *t = &trampoline->trampoline[0];                           \
        while (t->saved_callback) {                                                                \
            if (t->saved_callback == input) {                                                      \
                return (void *) t->thunk_instr;                                                    \
            }                                                                                      \
            t++;                                                                                   \
        }                                                                                          \
        t->saved_callback = input;                                                                 \
        return (void *) t->thunk_instr;                                                            \
    }



//
// Callback utils
//
#ifndef LORELIB_IS_GUEST_ADDR
#  define LORELIB_IS_GUEST_ADDR 1
#endif

struct LoreLib_CallbackContext {
    void **p_fp;
    void *org_fp;
};

#define LoreLib_CallbackContext_init(CTX, FP, ALLOCATOR)                                           \
    if (LORELIB_IS_GUEST_ADDR(FP)) {                                                               \
        (CTX).p_fp = (void **) &(FP);                                                              \
        (CTX).org_fp = FP;                                                                         \
        (FP) = ALLOCATOR(FP);                                                                      \
    } else {                                                                                       \
        (CTX).org_fp = NULL;                                                                       \
    }

#define LoreLib_CallbackContext_fini(CTX)                                                          \
    if ((CTX).org_fp) {                                                                            \
        *(CTX).p_fp = (CTX).org_fp;                                                                \
    }

#endif // LORELIB_COMMON_CALLBACK_H
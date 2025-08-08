#ifndef LORELIB_COMMON_CALLBACK_H
#define LORELIB_COMMON_CALLBACK_H

#define LORELIB_GET_LAST_CALLBACK_X86_64(NAME)                                                     \
    void *NAME;                                                                                    \
    asm volatile("mov %%r11, %0" : "=r"(NAME)::"memory");

#define LORELIB_GET_LAST_CALLBACK_ARM64(NAME)                                                      \
    void *NAME;                                                                                    \
    asm volatile("mov %0, x16" : "=r"(NAME)::"memory");

#define LORELIB_GET_LAST_CALLBACK_RISCV64(NAME)                                                    \
    void *NAME;                                                                                    \
    asm volatile("mv %0, t1" : "=r"(NAME)::"memory");


#define LORELIB_HCB_GET_LAST_CALLBACK LORELIB_GET_LAST_CALLBACK_X86_64

#ifdef __x86_64__
#  define LORELIB_GCB_GET_LAST_CALLBACK LORELIB_GET_LAST_CALLBACK_X86_64
#elif defined(__aarch64__)
#  define LORELIB_GCB_GET_LAST_CALLBACK LORELIB_GCB_GET_LAST_CALLBACK_ARM64
#elif defined(__riscv)
#  define LORELIB_GCB_GET_LAST_CALLBACK LORELIB_GCB_GET_LAST_CALLBACK_RISCV64
#endif

#define LORELIB_GCB_THUNK_ALLOCATOR(NAME)                                                          \
    static void *NAME##_GCB_THUNK_alloc(void *input) {                                             \
        static struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = NULL;                         \
        if (!trampoline) {                                                                         \
            trampoline = Lore_AllocCallbackTrampoline(1, __HTP_GCB_##NAME);                        \
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

#define LORELIB_HCB_THUNK_ALLOCATOR_DECL(NAME)                                                     \
    static void *NAME##_HCB_THUNK_alloc(void *input) {                                             \
        static struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = NULL;                         \
        if (!trampoline) {                                                                         \
            trampoline = Lore_AllocCallbackTrampoline(1, __GTP_HCB_##NAME);                        \
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

#endif // LORELIB_COMMON_CALLBACK_H
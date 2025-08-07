#ifndef LORELIB_COMMON_CALLBACK_H
#define LORELIB_COMMON_CALLBACK_H

// R10 stores the saved callback
#define LORELIB_GCB_THUNK_ASM_IMPL_X86_64(NAME)                                                    \
    __asm__(".text                                                  \n\t"                          \
            ".type    " #NAME "_GCB_THUNK_ASM, @function            \n\t"                          \
            "" #NAME "_GCB_THUNK_ASM:                               \n\t"                          \
            "mov    -8(%r10), %r10                                  \n\t"                          \
            "mov    Lore_HRTThreadCallback@gottpoff(%rip), %rax     \n\t"                          \
            "mov    %r10, %fs:(%rax)                                \n\t"                          \
            "jmp    __HTP_GCB_" #NAME);                                                            \
                                                                                                   \
    static inline void *NAME##_GCB_THUNK_ASM_addr() {                                              \
        void *addr;                                                                                \
        __asm__("lea " #NAME "_GCB_THUNK_ASM(%%rip), %0" : "=r"(addr));                            \
        return addr;                                                                               \
    }

// R10 stores the saved callback
#define LORELIB_HCB_THUNK_ASM_IMPL(NAME)                                                           \
    __asm__(".text                                                  \n\t"                          \
            ".type    " #NAME "_HCB_THUNK_ASM, @function            \n\t"                          \
            "" #NAME "_HCB_THUNK_ASM:                               \n\t"                          \
            "mov    -8(%r10), %r10                                  \n\t"                          \
            "mov    Lore_GRTThreadCallback@gottpoff(%rip), %rax     \n\t"                          \
            "mov    %r10, %fs:(%rax)                                \n\t"                          \
            "jmp    __GTP_HCB_" #NAME);                                                            \
                                                                                                   \
    static inline void *NAME##_HCB_THUNK_ASM_addr() {                                              \
        void *addr;                                                                                \
        __asm__("lea " #NAME "_HCB_THUNK_ASM(%%rip), %0" : "=r"(addr));                            \
        return addr;                                                                               \
    }

#ifdef __x86_64__
#  define LORELIB_GCB_THUNK_ASM_IMPL LORELIB_GCB_THUNK_ASM_IMPL_X86_64
#elif defined(__aarch64__)
#  define LORELIB_GCB_THUNK_ASM_IMPL LORELIB_GCB_THUNK_ASM_IMPL_ARM64
#elif defined(__riscv64)
#  define LORELIB_GCB_THUNK_ASM_IMPL LORELIB_GCB_THUNK_ASM_IMPL_RISCV64
#endif

#define LORELIB_GCB_THUNK_ASM_DECL(NAME) static inline void *NAME##_GCB_THUNK_ASM_get();
#define LORELIB_HCB_THUNK_ASM_DECL(NAME) static inline void *NAME##_HCB_THUNK_ASM_get();

#define LORELIB_GCB_THUNK_ASM_ALLOCATOR(NAME)                                                      \
    static void *NAME##_GCB_THUNK_alloc(void *input) {                                             \
        static struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = NULL;                         \
        if (!trampoline) {                                                                         \
            trampoline = Lore_AllocCallbackTrampoline(1, NAME##_GCB_THUNK_ASM_addr());             \
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

#define LORELIB_HCB_THUNK_ASM_ALLOCATOR_DECL(NAME)                                                 \
    static void *NAME##_HCB_THUNK_alloc(void *input) {                                             \
        static struct LORE_CALLBACK_TRAMPOLINE_CONTEXT *trampoline = NULL;                         \
        if (!trampoline) {                                                                         \
            trampoline = Lore_AllocCallbackTrampoline(1, NAME##_HCB_THUNK_ASM_addr());             \
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
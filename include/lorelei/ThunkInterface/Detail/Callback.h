// SPDX-License-Identifier: MIT

#ifndef LORE_THUNKINTERFACE_CALLBACK_H
#define LORE_THUNKINTERFACE_CALLBACK_H

#include <lorelei/DLCall/Tools/FunctionTrampoline.h>

// Now QEMU does not officially support address separation, use the fork QEMU instead.
#define QEMU_SUPPORT_ADDRESS_SEPARATION

#define LORE_THUNK_LAST_GCB lore::thread_last_callback

#define LORE_THUNK_LAST_HCB lore::thread_last_callback

// LORE_THUNK_GET_LAST_CALLBACK_*(NAME) read the host fixed register into NAME: it is where the
// generated callback trampoline leaves the address of the callback being invoked. That register
// (r11 on x86_64, x16 on aarch64, t1 on riscv64) is part of the trampoline ABI, so any translation
// unit that uses these macros (or the trampoline / callback-substitution path that relies on them)
// MUST reserve it with -ffixed-<reg>. Otherwise the compiler may reuse the register and overwrite
// the callback address before the read, and NAME picks up garbage.
#define LORE_THUNK_GET_LAST_CALLBACK_X86_64(NAME)                                                  \
    void *NAME;                                                                                    \
    asm volatile("mov %%r11, %0" : "=r"(NAME)::"memory");

#define LORE_THUNK_GET_LAST_CALLBACK_ARM64(NAME)                                                   \
    void *NAME;                                                                                    \
    asm volatile("mov %0, x16" : "=r"(NAME)::"memory");

#define LORE_THUNK_GET_LAST_CALLBACK_RISCV64(NAME)                                                 \
    void *NAME;                                                                                    \
    asm volatile("mv %0, t1" : "=r"(NAME)::"memory");

#define LORE_THUNK_GET_LAST_HCB LORE_THUNK_GET_LAST_CALLBACK_X86_64

#ifdef __x86_64__
#  define LORE_THUNK_GET_LAST_GCB LORE_THUNK_GET_LAST_CALLBACK_X86_64
#elif defined(__aarch64__)
#  define LORE_THUNK_GET_LAST_GCB LORE_THUNK_GET_LAST_CALLBACK_ARM64
#elif defined(__riscv)
#  define LORE_THUNK_GET_LAST_GCB LORE_THUNK_GET_LAST_CALLBACK_RISCV64
#endif

namespace lore {

    extern thread_local void *thread_last_callback;

}

namespace lore::thunk {

    static constexpr const size_t kMaxCallbackTrampolineCount = 16;

    /// Sentinel stamped into every trampoline block so guard code holding only a bare stub pointer can
    /// recognize it as one of ours and recover the original function (e.g. to revert a callback that
    /// has crossed back over the boundary). The value is UD2 (0F 0B) repeated: an invalid x86_64
    /// instruction stream, so it can never be mistaken for real code in the executable trampoline page
    /// and is extremely unlikely to collide with arbitrary data probed at the magic offset.
    static constexpr const uintptr_t kTrampolineMagic = 0x0B0F0B0F0B0F0B0FULL;

    /// GlobalTrampolineContext - Empty default \c Context for \c allocCallbackTrampoline; it
    /// selects a single shared (thread-local) trampoline table per callback signature rather than
    /// a per-context one.
    struct GlobalTrampolineContext {};

    template <auto F, class Context = GlobalTrampolineContext,
              size_t Count = kMaxCallbackTrampolineCount>
    static auto allocCallbackTrampoline(void *input) {
        using ReturnType = decltype(F);
        if (!input) {
            return (ReturnType) nullptr;
        }
        static thread_local lore::FunctionTrampolineTable *trampoline = nullptr;
        if (!trampoline) {
            trampoline = lore::FunctionTrampolineTable::create(Count, (void *) F, kTrampolineMagic);
        }
        auto t = &trampoline->trampoline[0];
        while (t->saved_function) {
            if (t->saved_function == input) {
                return (ReturnType) t->thunk_instr;
            }
            t++;
        }
        t->saved_function = input;
        return (ReturnType) t->thunk_instr;
    }

    // TODO: make it more like C++ style
    /// CallbackContext - Tracks one substituted callback: \c p_fp points at the slot holding the
    /// (trampolined) pointer and \c org_fp keeps the original function pointer.
    struct CallbackContext {
        void **p_fp;
        void *org_fp;
    };

    static inline bool isHostAddress(void *addr)
#ifndef LORE_THUNK_BUILD
    {
        return false;
    }
#else
        ; // implemented in 'ProcImpl.cpp.inc'
#endif

    template <bool isGuest, class F>
    static inline void CallbackContext_init(CallbackContext &ctx, void *&fp, F allocator) {
        if (isGuest != isHostAddress(fp)) {
            ctx.p_fp = &fp;
            ctx.org_fp = fp;
            *(void **) (&fp) = (void *) allocator(fp);
        } else {
            ctx.org_fp = NULL;
        }
    }

    static inline void CallbackContext_fini(CallbackContext &ctx) {
        if (ctx.org_fp) {
            *(ctx.p_fp) = ctx.org_fp;
        }
    }

}

#endif // LORE_THUNKINTERFACE_CALLBACK_H

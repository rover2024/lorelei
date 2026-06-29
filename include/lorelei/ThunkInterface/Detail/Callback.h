// SPDX-License-Identifier: MIT

#ifndef LORE_THUNKINTERFACE_CALLBACK_H
#define LORE_THUNKINTERFACE_CALLBACK_H

#include <lorelei/DLCall/Tools/FunctionTrampoline.h>

// Now QEMU does not officially support address separation, use the fork QEMU instead.
#define QEMU_SUPPORT_ADDRESS_SEPARATION

#define LORE_THUNK_LAST_GCB lore::thread_last_callback

#define LORE_THUNK_LAST_HCB lore::thread_last_callback

/// LORE_THUNK_GET_LAST_CALLBACK_*(NAME) read the host fixed register into NAME: it is where the
/// generated callback trampoline leaves the address of the callback being invoked. That register
/// (r11 on x86_64, x16 on aarch64, t1 on riscv64) is part of the trampoline ABI, so any translation
/// unit that uses these macros (or the trampoline / callback-substitution path that relies on them)
/// MUST reserve it with -ffixed-<reg>. Otherwise the compiler may reuse the register and overwrite
/// the callback address before the read, and NAME picks up garbage.
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

    /// Sentinel stamped into every trampoline block so guard code holding only a bare stub pointer
    /// can recognize it as one of ours and recover the original function (e.g. to revert a callback
    /// that has crossed back over the boundary). The value is UD2 (0F 0B) repeated: an invalid
    /// x86_64 instruction stream, so it can never be mistaken for real code in the executable
    /// trampoline page and is extremely unlikely to collide with arbitrary data probed at the magic
    /// offset.
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

    static inline bool isHostAddress(void *addr)
#ifndef LORE_THUNK_BUILD
    {
        return false;
    }
#else
        ; // implemented in 'ProcImpl.cpp.inc'
#endif

    /// If \a fp is one of our own trampoline stubs, return the original function it stands in for;
    /// otherwise return \a fp unchanged. Lets a callback that has crossed back over the boundary
    /// (one we handed out earlier, now passed or returned to us again) be reverted to its real
    /// address instead of being wrapped in a second trampoline. \c magic_sign sits past the stub,
    /// so the read stays inside the trampoline page for a real stub and inside the function's own
    /// code otherwise.
    static inline void *unwrapTrampoline(void *fp) {
        if (!fp) {
            return fp;
        }
        auto *t = reinterpret_cast<lore::FunctionTrampoline *>(
            reinterpret_cast<char *>(fp) - offsetof(lore::FunctionTrampoline, thunk_instr));
        if (t->magic_sign == kTrampolineMagic) {
            return t->saved_function;
        }
        return fp;
    }

    /// CallbackContext - Tracks one substituted callback so it can be wrapped before a call and
    /// restored after. \c init wraps a foreign callback in a receiver-callable trampoline and records
    /// the original at \c p_fp / \c org_fp; \c fini restores it. A default-constructed context (no
    /// init, or init that found nothing to wrap) carries a null \c org_fp and fini() is then a no-op.
    struct CallbackContext {
        void **p_fp = nullptr;
        void *org_fp = nullptr;

        /// Wrap \a fp so the side that will invoke it (the receiver) can call it, recording the
        /// original for \c fini to restore. \c isGuest is true when the callback belongs to the guest,
        /// so the receiver invoking it is then the host (and vice versa). \a allocator turns a pointer
        /// foreign to the receiver into a receiver-callable trampoline.
        template <bool isGuest, class Alloc>
        void init(void *&fp, Alloc allocator) {
            // Resolve fp to the real callback first: a stub we handed out earlier carries its origin.
            void *real = unwrapTrampoline(fp);
            if (isGuest != isHostAddress(real)) {
                // real is foreign to the receiver: hand over a receiver-callable trampoline for it.
                p_fp = &fp;
                org_fp = fp;
                fp = (void *) allocator(real);
            } else if (real != fp) {
                // fp was our stub but real is native to the receiver: pass the original through.
                p_fp = &fp;
                org_fp = fp;
                fp = real;
            }
            // else real is already native and not one of ours: nothing to wrap. org_fp keeps its
            // default null, so fini() is a no-op (same as a context whose init was guarded out).
        }

        /// Restore the caller's original pointer into the slot \c init wrapped. In callbacks call
        /// this after the call; out callbacks keep the wrapped value and never call it.
        void fini() {
            if (org_fp) {
                *p_fp = org_fp;
            }
        }
    };

}

#endif // LORE_THUNKINTERFACE_CALLBACK_H

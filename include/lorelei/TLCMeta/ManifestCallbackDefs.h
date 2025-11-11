#ifndef CALLBACKEXTRAS_H
#define CALLBACKEXTRAS_H

#include <lorelei/Core/ThunkTools/CallbackTrampoline.h>

#define LORETHUNK_LAST_GCB lore::thread_last_callback

#define LORETHUNK_LAST_HCB lore::thread_last_callback

#define LORETHUNK_GET_LAST_CALLBACK_X86_64(NAME)                                                   \
    void *NAME;                                                                                    \
    asm volatile("mov %%r11, %0" : "=r"(NAME)::"memory");

#define LORETHUNK_GET_LAST_CALLBACK_ARM64(NAME)                                                    \
    void *NAME;                                                                                    \
    asm volatile("mov %0, x16" : "=r"(NAME)::"memory");

#define LORETHUNK_GET_LAST_CALLBACK_RISCV64(NAME)                                                  \
    void *NAME;                                                                                    \
    asm volatile("mv %0, t1" : "=r"(NAME)::"memory");

#define LORETHUNK_GET_LAST_HCB LORETHUNK_GET_LAST_CALLBACK_X86_64

#ifdef __x86_64__
#  define LORETHUNK_GET_LAST_GCB LORETHUNK_GET_LAST_CALLBACK_X86_64
#elif defined(__aarch64__)
#  define LORETHUNK_GET_LAST_GCB LORETHUNK_GET_LAST_CALLBACK_ARM64
#elif defined(__riscv)
#  define LORETHUNK_GET_LAST_GCB LORETHUNK_GET_LAST_CALLBACK_RISCV64
#endif

namespace lore {

    extern thread_local void *thread_last_callback;

}

namespace lorethunk {

    static constexpr const size_t kMaxCallbackTrampolineCount = 16;

    struct GlobalTramplolineContext {};

    template <auto F, class Context = GlobalTramplolineContext,
              size_t Count = kMaxCallbackTrampolineCount>
    static auto allocCallbackTrampoline(void *input) {
        using ReturnType = decltype(F);
        static thread_local lore::CallbackTrampolineTable *trampoline = nullptr;
        if (!trampoline) {
            trampoline = lore::CallbackTrampolineTable::create(Count, (void *) F);
        }
        auto t = &trampoline->trampoline[0];
        while (t->saved_callback) {
            if (t->saved_callback == input) {
                return (ReturnType) t->thunk_instr;
            }
            t++;
        }
        t->saved_callback = input;
        return (ReturnType) t->thunk_instr;
    }

    // TODO: make it more like C++ style
    struct CallbackContext {
        void **p_fp;
        void *org_fp;
    };

    static inline bool isHostAddress(void *addr)
#ifndef LORETHUNK_BUILD
    {
        return false;
    }
#else
        ; // implemented in 'ManifestContext_<plat>_impl.inc.h'
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

#endif // CALLBACKEXTRAS_H

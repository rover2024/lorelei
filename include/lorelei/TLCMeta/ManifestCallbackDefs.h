#ifndef CALLBACKEXTRAS_H
#define CALLBACKEXTRAS_H

#include <lorelei/Core/ThunkTools/CallbackTrampoline.h>

#define LORETHUNK_LAST_GCB nullptr

#define LORETHUNK_LAST_HCB nullptr

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

namespace lorethunk {

    static constexpr const size_t kMaxCallbackTrampolineCount = 16;

    template <auto F, size_t Count = kMaxCallbackTrampolineCount>
    static auto allocCallbackTrampoline(void *input) {
        static thread_local lore::CallbackTrampolineTable *trampoline = nullptr;
        if (!trampoline) {
            trampoline = lore::CallbackTrampolineTable::create(Count, (void *) F);
        }
        auto t = &trampoline->trampoline[0];
        while (t->saved_callback) {
            if (t->saved_callback == input) {
                return (decltype(F)) t->thunk_instr;
            }
            t++;
        }
        t->saved_callback = input;
        return (decltype(F)) t->thunk_instr;
    }

}

#endif // CALLBACKEXTRAS_H

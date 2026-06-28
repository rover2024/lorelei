// SPDX-License-Identifier: MIT

#include "Invocation.h"

#include <pthread.h>
#include <dlfcn.h>

#include <memory>
#include <cstddef>
#include <cassert>
#include <cstring>

// #define LORE_USE_EMU_TASK_ENTRY

extern "C" {

#ifdef __x86_64__
#  include "Arch/x86_64/RegState_x86_64.h"
#elif defined(__aarch64__)
#  include "Arch/aarch64/RegState_aarch64.h"
#elif defined(__riscv) && __riscv_xlen == 64
#  include "Arch/riscv64/RegState_riscv64.h"
#else
#  error "Unsupported architecture"
#endif

extern int64_t coroutine_start(void *arg1, void *arg2, int64_t (*proc)(void *, void *),
                               void *new_stack, RegState *from);

extern int64_t coroutine_switch(RegState *from, RegState *to, int64_t ret);
}

namespace lore::utils {

    using InvocationArguments = Invocation::InvocationArguments;
    using ReentryArguments = Invocation::ReentryArguments;

    struct HostExecContext {
        std::unique_ptr<char[]> stack;
        size_t stackSize;
        char *stackTop;

        struct InvocationInfo {
            ReentryArguments **ra_ptr;
            RegState *hostState;
        };
        InvocationInfo *invocations;
        size_t invocationCount;

        RegState mainHostState;

        HostExecContext();
        ~HostExecContext();

        inline InvocationInfo &lastInvocation() {
            assert(invocationCount > 0);
            return invocations[invocationCount - 1];
        }

        inline void pushInvocation(ReentryArguments **ra_ptr, RegState *hostState) {
            assert(ra_ptr);
            assert(hostState);
            assert(invocationCount < (stackSize / sizeof(InvocationInfo)));
            invocations[invocationCount++] = {
                ra_ptr,
                hostState,
            };
        }

        inline void popInvocation() {
            assert(invocationCount > 0);
            invocationCount--;
        }

        static int64_t invocationEntry(void *arg1, void *arg2);
    };

    static thread_local HostExecContext thread_ctx;

    static inline size_t get_default_stack_size() {
        pthread_attr_t attr;
        size_t stack_size;

        pthread_attr_init(&attr);
        pthread_attr_getstacksize(&attr, &stack_size);
        pthread_attr_destroy(&attr);

        return stack_size;
    }

    HostExecContext::HostExecContext() {
        static size_t default_stack_size = get_default_stack_size();

        stackSize = default_stack_size;
        stack = std::make_unique<char[]>(stackSize);
        // Coroutine stacks grow downward, so the highest address is the top. Align it down to a
        // 16-byte boundary as required by the SysV/AAPCS call ABIs.
        stackTop = reinterpret_cast<char *>(
            reinterpret_cast<uintptr_t>(stack.get() + stackSize) & ~uintptr_t(0xF));

        // The invocation stack grows up from the buffer bottom while the coroutine call stack grows
        // down from stackTop, so the two share one buffer from opposite ends.
        invocations = reinterpret_cast<InvocationInfo *>(stack.get());
        invocationCount = 0;
    }

    HostExecContext::~HostExecContext() = default;

    int64_t Invocation::invoke(const InvocationArguments *ia, ReentryArguments **ra_ptr) {
#ifdef LORE_USE_EMU_TASK_ENTRY
        return HostExecContext::invocationEntry(const_cast<InvocationArguments *>(ia), ra_ptr);
#else
        // A nested invocation must not clobber the suspended one below it: start fresh at stackTop
        // only when nothing is live, otherwise carve out below the suspended invocation's saved SP.
        auto stack = thread_ctx.invocationCount == 0
                         ? reinterpret_cast<uintptr_t>(thread_ctx.stackTop)
                         : (RegStateGetSP(thread_ctx.lastInvocation().hostState) & ~uintptr_t(0xF));
        return coroutine_start(const_cast<InvocationArguments *>(ia), ra_ptr,
                               HostExecContext::invocationEntry, reinterpret_cast<void *>(stack),
                               &thread_ctx.mainHostState);
#endif
    }

    int64_t Invocation::resume() {
        assert(thread_ctx.invocationCount > 0);
        // Switch into the suspended invocation. The value returned here is whatever the invocation
        // hands back when it next yields: 1 if it suspended at another reentry, 0 if it finished.
        return coroutine_switch(&thread_ctx.mainHostState, thread_ctx.lastInvocation().hostState,
                                0);
    }

    void Invocation::reenter(ReentryArguments *ra) {
        assert(ra);
        assert(thread_ctx.invocationCount > 0);
        auto &last = thread_ctx.lastInvocation();
        *last.ra_ptr = ra;

#ifdef LORE_USE_EMU_TASK_ENTRY
        static auto entry = []() {
            auto entry = dlsym(nullptr, "lorelei_run_task_entry");
            if (!entry) {
                abort();
            }
            return (void (*)(void)) entry;
        }();
        entry();
#else
        // Suspend this invocation and yield 1 to the driver to signal "stopped at a reentry". The
        // saved hostState lets resume() switch back here once the reentry has been serviced.
        std::ignore = coroutine_switch(last.hostState, &thread_ctx.mainHostState, 1);
#endif
    }

    int64_t HostExecContext::invocationEntry(void *arg1, void *arg2) {
        // state lives on this coroutine's stack and is registered as the invocation's saved context.
        // It stays valid for the whole call because the coroutine stack is never unwound while the
        // invocation is merely suspended; it is only torn down once invokeByConv returns below.
        RegState state;
        thread_ctx.pushInvocation(reinterpret_cast<ReentryArguments **>(arg2), &state);

        Invocation::invokeByConv(reinterpret_cast<const InvocationArguments *>(arg1));

        thread_ctx.popInvocation();
        return 0;
    }

}

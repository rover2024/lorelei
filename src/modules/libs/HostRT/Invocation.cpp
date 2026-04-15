#include "Invocation.h"

#include <pthread.h>
#include <dlfcn.h>

#include <memory>
#include <cstddef>
#include <cassert>
#include <cstring>

#include <lorelei/Base/PassThrough/Core/IClient.h>
#include <lorelei/Base/PassThrough/Core/SyscallPassThrough.h>
#include <lorelei/Base/PassThrough/ThunkTools/VariadicAdaptor.h>

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

namespace lore::mod {

    static inline int invoke_by_conv(const InvocationArguments *ia) {
        switch (ia->conv) {
            case CC_Standard: {
                using Func = void (*)(void ** /*args*/, void * /*ret*/, void * /*metadata*/);

                auto func = reinterpret_cast<Func>(ia->standard.proc);
                func(ia->standard.args, ia->standard.ret, ia->standard.metadata);
                return 0;
            }

            case CC_StandardCallback: {
                using Func = void (*)(void * /*callback*/, void ** /*args*/, void * /*ret*/,
                                      void * /*metadata*/);

                auto func = reinterpret_cast<Func>(ia->standardCallback.proc);
                func(ia->standardCallback.callback, ia->standardCallback.args,
                     ia->standardCallback.ret, ia->standardCallback.metadata);
                return 0;
            }

            case CC_Format: {
                VariadicAdaptor::callFormatBox64(ia->format.proc, ia->format.format,
                                                 ia->format.args, ia->format.ret);
                return 0;
            }

            case CC_ThreadEntry: {
                using Func = void *(*) (void *);

                auto func = reinterpret_cast<Func>(ia->threadEntry.proc);
                *ia->threadEntry.ret = func(ia->threadEntry.arg);
                return 0;
            }

            default:
                return -1;
        }
    }

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
        stackTop = (char *) (((uint64_t) (stack.get() + stackSize)) & ~0xFULL);

        // Use stack bottom as call info array.
        invocations = reinterpret_cast<InvocationInfo *>(stack.get());
        invocationCount = 0;
    }

    HostExecContext::~HostExecContext() = default;

    int64_t Invocation::invoke(const InvocationArguments *ia, ReentryArguments **ra_ptr) {
#ifdef LORE_USE_EMU_TASK_ENTRY
        return HostExecContext::invocationEntry(const_cast<InvocationArguments *>(ia), ra_ptr);
#else
        auto stack = thread_ctx.invocationCount == 0
                         ? (uintptr_t) thread_ctx.stackTop
                         : (RegStateGetSP(thread_ctx.lastInvocation().hostState) & ~0xFULL);
        return coroutine_start(const_cast<InvocationArguments *>(ia), ra_ptr,
                               HostExecContext::invocationEntry, (void *) stack,
                               &thread_ctx.mainHostState);
#endif
    }

    int64_t Invocation::resume() {
        assert(thread_ctx.invocationCount > 0);
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
        std::ignore = coroutine_switch(last.hostState, &thread_ctx.mainHostState, 1);
#endif
    }

    int64_t HostExecContext::invocationEntry(void *arg1, void *arg2) {
        RegState state;
        thread_ctx.pushInvocation(reinterpret_cast<ReentryArguments **>(arg2), &state);

        invoke_by_conv(reinterpret_cast<const InvocationArguments *>(arg1));

        thread_ctx.popInvocation();
        return 0;
    }

}

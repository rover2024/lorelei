#ifndef LOREGUESTRT_X86_64_SYSCALL_H
#define LOREGUESTRT_X86_64_SYSCALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t syscall1(uint64_t num, uint64_t arg1) {
#ifdef __x86_64__
    uint64_t ret;
    asm volatile("movq %1, %%rax\n"
                 "movq %2, %%rdi\n"
                 "syscall\n"
                 "movq %%rax, %0\n"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1)
                 : "%rax", "%rdi", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall2(uint64_t num, uint64_t arg1, uint64_t arg2) {
#ifdef __x86_64__
    uint64_t ret;
    asm volatile("mov %1, %%rax\n"
                 "mov %2, %%rdi\n"
                 "mov %3, %%rsi\n"
                 "syscall\n"
                 "mov %%rax, %0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2)
                 : "%rax", "%rdi", "%rsi", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
#ifdef __x86_64__
    uint64_t ret;
    asm volatile("mov %1, %%rax\n"
                 "mov %2, %%rdi\n"
                 "mov %3, %%rsi\n"
                 "mov %4, %%rdx\n"
                 "syscall\n"
                 "mov %%rax, %0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3)
                 : "%rax", "%rdi", "%rsi", "%rdx", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall4(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4) {
#ifdef __x86_64__
    uint64_t ret;
    asm volatile("mov %1, %%rax\n"
                 "mov %2, %%rdi\n"
                 "mov %3, %%rsi\n"
                 "mov %4, %%rdx\n"
                 "mov %5, %%r10\n"
                 "syscall\n"
                 "mov %%rax, %0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
                 : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall5(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4, uint64_t arg5) {
#ifdef __x86_64__
    uint64_t ret;
    asm volatile("mov %1, %%rax\n"
                 "mov %2, %%rdi\n"
                 "mov %3, %%rsi\n"
                 "mov %4, %%rdx\n"
                 "mov %5, %%r10\n"
                 "mov %6, %%r8\n"
                 "syscall\n"
                 "mov %%rax, %0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5)
                 : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall6(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4, uint64_t arg5, uint64_t arg6) {
#ifdef __x86_64__
    uint64_t ret;
    asm volatile("mov %1, %%rax\n"
                 "mov %2, %%rdi\n"
                 "mov %3, %%rsi\n"
                 "mov %4, %%rdx\n"
                 "mov %5, %%r10\n"
                 "mov %6, %%r8\n"
                 "mov %7, %%r9\n"
                 "syscall\n"
                 "mov %%rax, %0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
                 : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "%r8", "%r9", "memory");
    return ret;
#else
    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#endif // LOREGUESTRT_X86_64_SYSCALL_H

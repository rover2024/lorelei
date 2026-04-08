#ifndef LORE_MODULES_GUESTRT_AARCH64_SYSCALL_H
#define LORE_MODULES_GUESTRT_AARCH64_SYSCALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t syscall1(uint64_t num, uint64_t arg1) {
#ifdef __aarch64__
    uint64_t ret;
    asm volatile("mov x8, %1\n"
                 "mov x0, %2\n"
                 "svc #0\n"
                 "mov %0, x0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1)
                 : "x8", "x0", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall2(uint64_t num, uint64_t arg1, uint64_t arg2) {
#ifdef __aarch64__
    uint64_t ret;
    asm volatile("mov x8, %1\n"
                 "mov x0, %2\n"
                 "mov x1, %3\n"
                 "svc #0\n"
                 "mov %0, x0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2)
                 : "x8", "x0", "x1", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
#ifdef __aarch64__
    uint64_t ret;
    asm volatile("mov x8, %1\n"
                 "mov x0, %2\n"
                 "mov x1, %3\n"
                 "mov x2, %4\n"
                 "svc #0\n"
                 "mov %0, x0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3)
                 : "x8", "x0", "x1", "x2", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall4(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4) {
#ifdef __aarch64__
    uint64_t ret;
    asm volatile("mov x8, %1\n"
                 "mov x0, %2\n"
                 "mov x1, %3\n"
                 "mov x2, %4\n"
                 "mov x3, %5\n"
                 "svc #0\n"
                 "mov %0, x0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
                 : "x8", "x0", "x1", "x2", "x3", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall5(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4, uint64_t arg5) {
#ifdef __aarch64__
    uint64_t ret;
    asm volatile("mov x8, %1\n"
                 "mov x0, %2\n"
                 "mov x1, %3\n"
                 "mov x2, %4\n"
                 "mov x3, %5\n"
                 "mov x4, %6\n"
                 "svc #0\n"
                 "mov %0, x0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5)
                 : "x8", "x0", "x1", "x2", "x3", "x4", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall6(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4, uint64_t arg5, uint64_t arg6) {
#ifdef __aarch64__
    uint64_t ret;
    asm volatile("mov x8, %1\n"
                 "mov x0, %2\n"
                 "mov x1, %3\n"
                 "mov x2, %4\n"
                 "mov x3, %5\n"
                 "mov x4, %6\n"
                 "mov x5, %7\n"
                 "svc #0\n"
                 "mov %0, x0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
                 : "x8", "x0", "x1", "x2", "x3", "x4", "x5", "memory");
    return ret;
#else
    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#endif // LORE_MODULES_GUESTRT_AARCH64_SYSCALL_H

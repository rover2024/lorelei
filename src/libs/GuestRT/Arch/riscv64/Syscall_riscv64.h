#ifndef LOREGUESTRT_RISCV64_SYSCALL_H
#define LOREGUESTRT_RISCV64_SYSCALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t syscall1(uint64_t num, uint64_t arg1) {
#ifdef __riscv
    uint64_t ret;
    asm volatile("mv a7, %1\n"
                 "mv a0, %2\n"
                 "ecall\n"
                 "mv %0, a0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1)
                 : "a7", "a0", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall2(uint64_t num, uint64_t arg1, uint64_t arg2) {
#ifdef __riscv
    uint64_t ret;
    asm volatile("mv a7, %1\n"
                 "mv a0, %2\n"
                 "mv a1, %3\n"
                 "ecall\n"
                 "mv %0, a0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2)
                 : "a7", "a0", "a1", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall3(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
#ifdef __riscv
    uint64_t ret;
    asm volatile("mv a7, %1\n"
                 "mv a0, %2\n"
                 "mv a1, %3\n"
                 "mv a2, %4\n"
                 "ecall\n"
                 "mv %0, a0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3)
                 : "a7", "a0", "a1", "a2", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall4(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4) {
#ifdef __riscv
    uint64_t ret;
    asm volatile("mv a7, %1\n"
                 "mv a0, %2\n"
                 "mv a1, %3\n"
                 "mv a2, %4\n"
                 "mv a3, %5\n"
                 "ecall\n"
                 "mv %0, a0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
                 : "a7", "a0", "a1", "a2", "a3", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall5(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4, uint64_t arg5) {
#ifdef __riscv
    uint64_t ret;
    asm volatile("mv a7, %1\n"
                 "mv a0, %2\n"
                 "mv a1, %3\n"
                 "mv a2, %4\n"
                 "mv a3, %5\n"
                 "mv a4, %6\n"
                 "ecall\n"
                 "mv %0, a0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5)
                 : "a7", "a0", "a1", "a2", "a3", "a4", "memory");
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall6(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                uint64_t arg4, uint64_t arg5, uint64_t arg6) {
#ifdef __riscv
    uint64_t ret;
    asm volatile("mv a7, %1\n"
                 "mv a0, %2\n"
                 "mv a1, %3\n"
                 "mv a2, %4\n"
                 "mv a3, %5\n"
                 "mv a4, %6\n"
                 "mv a5, %7\n"
                 "ecall\n"
                 "mv %0, a0"
                 : "=r"(ret)
                 : "r"(num), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "r"(arg5), "r"(arg6)
                 : "a7", "a0", "a1", "a2", "a3", "a4", "a5", "memory");
    return ret;
#else
    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#endif // LOREGUESTRT_RISCV64_SYSCALL_H

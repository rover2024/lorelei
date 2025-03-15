#ifndef SYSCALL_HELPER_H
#define SYSCALL_HELPER_H

#include <stdint.h>

static inline uint64_t syscall1(uint64_t syscall_number, void *arg) {
#ifdef __x86_64__
    uint64_t ret;
    __asm__ volatile("movq %1, %%rax\n"              // 加载系统调用号到 rax
                     "movq %2, %%rdi\n"              // 加载参数到 rdi
                     "syscall\n"                     // 执行系统调用
                     "movq %%rax, %0\n"              // 将返回值存储在 ret 中
                     : "=r"(ret)                     // 输出列表
                     : "r"(syscall_number), "r"(arg) // 输入列表
                     : "%rax", "%rdi", "memory"      // 被改变的寄存器列表
    );
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall2(uint64_t syscall_number, void *arg1, void *arg2) {
#ifdef __x86_64__
    uint64_t ret;
    __asm__ volatile("mov %1, %%rax\n"                           // 系统调用号放入rax
                     "mov %2, %%rdi\n"                           // 第一个参数arg1放入rdi
                     "mov %3, %%rsi\n"                           // 第二个参数arg2放入rsi
                     "syscall\n"                                 // 执行系统调用
                     "mov %%rax, %0"                             // 系统调用返回值放入result
                     : "=r"(ret)                                 // 输出操作数
                     : "r"(syscall_number), "r"(arg1), "r"(arg2) // 输入操作数
                     : "%rax", "%rdi", "%rsi", "memory"          // 被修改寄存器列表
    );
    return ret;
#else
    return 0;
#endif
}

static inline uint64_t syscall3(uint64_t syscall_number, void *arg1, void *arg2, void *arg3) {
#ifdef __x86_64__
    uint64_t ret;
    __asm__ volatile("mov %1, %%rax\n" // 系统调用号放入rax
                     "mov %2, %%rdi\n" // 第一个参数arg1放入rdi
                     "mov %3, %%rsi\n" // 第二个参数arg2放入rsi
                     "mov %4, %%rdx\n" // 第三个参数arg3放入rdx
                     "syscall\n"       // 执行系统调用
                     "mov %%rax, %0"   // 系统调用返回值放入result
                     : "=r"(ret)       // 输出操作数
                     : "r"(syscall_number), "r"((uint64_t) arg1), "r"((uint64_t) arg2),
                       "r"((uint64_t) arg3)                     // 输入操作数
                     : "%rax", "%rdi", "%rsi", "%rdx", "memory" // 被修改寄存器列表
    );
    return ret;
#else
    return 0;
#endif
}


static inline uint64_t syscall4(uint64_t syscall_number, void *arg1, void *arg2, void *arg3, void *arg4) {
#ifdef __x86_64__
    uint64_t ret;
    __asm__ volatile("mov %1, %%rax\n" // 系统调用号放入rax
                     "mov %2, %%rdi\n" // 第一个参数arg1放入rdi
                     "mov %3, %%rsi\n" // 第二个参数arg2放入rsi
                     "mov %4, %%rdx\n" // 第三个参数arg3放入rdx
                     "mov %5, %%r10\n" // 第四个参数arg4放入r10
                     "syscall\n"       // 执行系统调用
                     "mov %%rax, %0"   // 系统调用返回值放入result
                     : "=r"(ret)       // 输出操作数
                     : "r"(syscall_number), "r"((uint64_t) arg1), "r"((uint64_t) arg2), "r"((uint64_t) arg3),
                       "r"((uint64_t) arg4)                             // 输入操作数
                     : "%rax", "%rdi", "%rsi", "%rdx", "%r10", "memory" // 被修改寄存器列表
    );
    return ret;
#else
    return 0;
#endif
}

#endif // SYSCALL_HELPER_H
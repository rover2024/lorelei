#include <stdint.h>

struct RegState {
    uintptr_t rbx;
    uintptr_t rsp;
    uintptr_t rbp;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
    uintptr_t rip; // not used
};

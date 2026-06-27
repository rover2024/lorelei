// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>

/// RegState - Callee-saved registers plus the stack and return pointers captured for a
/// TinyCoroutine context switch.
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

uintptr_t RegStateGetSP(const struct RegState *reg) {
    return reg->rsp;
}

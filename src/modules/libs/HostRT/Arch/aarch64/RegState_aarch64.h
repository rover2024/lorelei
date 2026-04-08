#pragma once

#include <cstdint>

struct RegState {
    uintptr_t x19;
    uintptr_t x20;
    uintptr_t x21;
    uintptr_t x22;
    uintptr_t x23;
    uintptr_t x24;
    uintptr_t x25;
    uintptr_t x26;
    uintptr_t x27;
    uintptr_t x28;
    uintptr_t x29;
    uintptr_t sp;
    uintptr_t lr;
    uintptr_t pc; // not used
};

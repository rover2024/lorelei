#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void qemu_magic_ud2(uint32_t op, uint32_t tag) {
    register uint32_t eax __asm__("eax") = op;
    register uint32_t ebx __asm__("ebx") = tag;
    __asm__ volatile("ud2" : "+a"(eax), "+b"(ebx) : : "memory");
}

static inline void gtl_start() {
    qemu_magic_ud2(1u, 0xA001u);
}

static inline void gtl_end() {
    qemu_magic_ud2(2u, 0xA001u);
}

#ifdef __cplusplus
}
#endif

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Generate:
//     Save current Entry to X16 and jump to address Entry-N
static void tramp_gen_thunk(void *buf, int64_t N) {
    uint32_t *instr = (uint32_t *) buf;

    // _start:
    // ADR X16, #0
    instr[0] = 0x10000010;

    // target address = buf - N
    // branch instruction address = buf + 4
    // actual offset = (buf - N) - (buf + 4) = -4 - N
    // instruction offset = (-4 - N) / 4
    int32_t jump_offset = (-4 - (int32_t) N) / 4;

    // B <offset>
    instr[1] = 0x14000000 | (jump_offset & 0x03FFFFFF);
}

// Generate:
//     Jump to address Entry
static void tramp_gen_jump(void *buf, void *target) {
    struct instr {
        uint32_t ldr_x16;
        uint32_t ldr_x17;
        uint32_t br_x17;
        uint64_t target_addr;
    } __attribute__((packed));
    struct instr *instr = (struct instr *) buf;
    instr->ldr_x16 = 0xF85F8210; // ldr x16, [x16, #-8]
    instr->ldr_x17 = 0x58000051; // ldr x17, 8 <pc+8>
    instr->br_x17 = 0xD61F0220;  // br x17
    instr->target_addr = (uint64_t) target;
}

#ifdef __cplusplus
}
#endif

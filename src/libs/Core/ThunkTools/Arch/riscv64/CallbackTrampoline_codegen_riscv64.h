#ifndef LORECORE_CALLBACKTRAMPOLINE_CODEGEN_RISCV64_H
#define LORECORE_CALLBACKTRAMPOLINE_CODEGEN_RISCV64_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Generate:
//     Save current Entry to t1 and jump to address Entry-N
static void cbt_gen_thunk(char *buf, int64_t N) {
    uint32_t *instr = (uint32_t *) buf;
    int32_t imm = -4 - (int32_t) N;

    // _start:
    // auipc t1, 0
    instr[0] = 0x00000317;

    // jal x0, offset
    uint32_t imm20 = ((imm >> 20) & 0x1) << 31;
    uint32_t imm10_1 = ((imm >> 1) & 0x3FF) << 21;
    uint32_t imm11 = ((imm >> 11) & 0x1) << 20;
    uint32_t imm19_12 = ((imm >> 12) & 0xFF) << 12;

    uint32_t rd = 0;        // x0
    uint32_t opcode = 0x6F; // JAL opcode
    instr[1] = imm20 | imm19_12 | imm11 | imm10_1 | (rd << 7) | opcode;
}

// Generate:
//     Jump to address Entry
static void cbt_gen_jump(void *buf, void *target) {
    struct instr {
        uint32_t ld_t1;
        uint32_t auipc_t0;
        uint32_t ld_t0_12;
        uint32_t jr_t0;
        uint64_t target_addr;
    } __attribute__((packed));
    struct instr *instr = (struct instr *) buf;
    instr->ld_t1 = 0xFF833303;    // ld t1, -8(t1)
    instr->auipc_t0 = 0x00000297; // auipc t0, 0
    instr->ld_t0_12 = 0x00C2B283; // ld t0, 12(t0)
    instr->jr_t0 = 0x00028067;    // jr t0
    instr->target_addr = (uint64_t) target;
}

#ifdef __cplusplus
}
#endif

#endif // LORECORE_CALLBACKTRAMPOLINE_CODEGEN_RISCV64_H

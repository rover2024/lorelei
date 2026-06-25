#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Generate:
//     Save current Entry to R11 and jump to address Entry-N
static void tramp_gen_thunk(void *buf, int64_t N) {
    struct instr {
        struct {
            char op[3];
            int32_t displacement;
        } __attribute__((packed)) lea; // leaq -N(%rip), %r11
        struct {
            char op[1];
            int32_t offset;
        } __attribute__((packed)) jmp_rel32; // jmp N(%rip)
    } __attribute__((packed));

    struct instr *instr = (struct instr *) buf;
    instr->lea.op[0] = 0x4C; // REX.W prefix
    instr->lea.op[1] = 0x8D; // LEA opcode
    instr->lea.op[2] = 0x1D; // ModR/M: r11 with RIP-relative addressing
    instr->lea.displacement = -7;

    instr->jmp_rel32.op[0] = 0xE9;     // JMP opcode
    instr->jmp_rel32.offset = -12 - N; // N bytes from last instruction
}

// Generate:
//     Jump to address Entry
static void tramp_gen_jump(void *buf, void *target) {
    struct instr {
        char mov[4]; // mov -0x8(%r11),%r11
        struct {
            char op[2];
            uint64_t imm;
        } __attribute__((packed)) mov_target; // movabs $target, %rax
        char jmp[2];                          // jmp *%rax
    } __attribute__((packed));

    struct instr *instr = (struct instr *) buf;
    instr->mov[0] = 0x4D; // REX.W prefix
    instr->mov[1] = 0x8B; // MOV opcode
    instr->mov[2] = 0x5B; // ModR/M: r11 with RIP-relative addressing
    instr->mov[3] = 0xF8; // -8

    instr->mov_target.op[0] = 0x48;            // REX.W prefix
    instr->mov_target.op[1] = 0xB8;            // MOV opcode
    instr->mov_target.imm = (uint64_t) target; // target address

    instr->jmp[0] = 0xFF; // JMP opcode
    instr->jmp[1] = 0xE0; // ModR/M: indirect jump with RAX
}

#ifdef __cplusplus
}
#endif

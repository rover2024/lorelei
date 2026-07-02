// SPDX-License-Identifier: MIT
//
// The shared trampoline shim. A per-instance stub tail-jumps here with
//     %r11 = the identity (saved_function),  %r10 = the handler to run.
// It parks the identity in the thread_last_callback thread-local and tail-jumps to the handler.
//
// thread_last_callback is an initial-exec thread-local, so the store is a call-free TP-relative move,
// with no __tls_get_addr to clobber the argument registers. The @gottpoff relocation is resolved by
// the toolchain and loader, so we never compute the TLS offset ourselves. Nothing here touches an
// argument register or the stack, so the handler sees every argument, including the stack-passed
// ones, exactly as the caller set them.

    .text
    .globl lore_tramp_shim
    .type lore_tramp_shim, @function
lore_tramp_shim:
    mov thread_last_callback@gottpoff(%rip), %rax   // %rax = TP offset of thread_last_callback
    mov %r11, %fs:(%rax)                            // thread_last_callback = identity
    jmp *%r10                                       // tail-branch to the handler
    .size lore_tramp_shim, . - lore_tramp_shim

    .section .note.GNU-stack, "", @progbits

// SPDX-License-Identifier: MIT
//
// The shared trampoline shim (aarch64). A per-instance stub tail-branches here with
//     x16 = the identity (saved_function),  x17 = the handler to run.
// It parks the identity in the thread_last_callback thread-local and tail-branches to the handler.
//
// thread_last_callback is an initial-exec thread-local, so the store is a call-free TP-relative move,
// with no __tls_get_addr to clobber the argument registers. The GOTTPREL relocations are resolved by
// the toolchain and loader. Only x8/x9 (temporaries) and x16/x17 (the IP registers the stub set) are
// touched, so the argument registers x0-x7 (and the FP argument registers) reach the handler exactly
// as the caller passed them, including anything on the stack.

    .text
    .globl lore_tramp_shim
    .type lore_tramp_shim, %function
lore_tramp_shim:
    adrp x9, :gottprel:thread_last_callback
    ldr  x9, [x9, #:gottprel_lo12:thread_last_callback]   // x9 = TP offset of thread_last_callback
    mrs  x8, tpidr_el0                                    // x8 = thread pointer
    str  x16, [x8, x9]                                    // thread_last_callback = identity
    br   x17                                              // tail-branch to the handler
    .size lore_tramp_shim, . - lore_tramp_shim

    .section .note.GNU-stack, "", %progbits

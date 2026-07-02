// SPDX-License-Identifier: MIT
//
// The shared trampoline shim (riscv64). A per-instance stub tail-branches here with
//     t0 = the identity (saved_function),  t1 = the handler to run.
// It parks the identity in the thread_last_callback thread-local and tail-branches to the handler.
//
// thread_last_callback is an initial-exec thread-local, so la.tls.ie resolves it through the GOT
// (R_RISCV_TLS_GOT_HI20) into a call-free TP-relative store, with no __tls_get_addr to clobber the
// argument registers. Only t0/t1/t2 (temporaries the stub set or we scratch) and tp are touched, so
// the argument registers a0-a7 (and the FP argument registers) reach the handler exactly as the
// caller passed them, including anything on the stack.

    .option norvc
    .text
    .globl lore_tramp_shim
    .type lore_tramp_shim, @function
lore_tramp_shim:
    la.tls.ie t2, thread_last_callback  // t2 = TP offset of thread_last_callback (via the GOT)
    add t2, t2, tp                      // t2 = &thread_last_callback in this thread
    sd  t0, 0(t2)                       // thread_last_callback = identity
    jr  t1                              // tail-branch to the handler
    .size lore_tramp_shim, . - lore_tramp_shim

    .section .note.GNU-stack, "", @progbits

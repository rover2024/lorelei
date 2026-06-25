.text

// int64_t coroutine_start(
//      void *arg1,
//      void *arg2,
//      int64_t (*proc)(void *, void *),
//      void *new_stack,
//      RegState *from
//  )
.global coroutine_start
.type coroutine_start, %function
.hidden coroutine_start
coroutine_start:
    // x0 = arg1
    // x1 = arg2
    // x2 = proc
    // x3 = new_stack
    // x4 = from

    // save callee-saved registers and context to "from"
    stp x19, x20, [x4, #0]
    stp x21, x22, [x4, #16]
    stp x23, x24, [x4, #32]
    stp x25, x26, [x4, #48]
    stp x27, x28, [x4, #64]
    stp x29, x30, [x4, #80]
    mov x9, sp
    str x9, [x4, #96]

    // switch stack
    mov sp, x3

    // keep from pointer across the call
    mov x19, x4

    // call proc(arg1, arg2)
    blr x2

    // restore from pointer
    mov x4, x19

    // restore callee-saved registers and stack
    ldp x19, x20, [x4, #0]
    ldp x21, x22, [x4, #16]
    ldp x23, x24, [x4, #32]
    ldp x25, x26, [x4, #48]
    ldp x27, x28, [x4, #64]
    ldp x29, x30, [x4, #80]
    ldr x9, [x4, #96]
    mov sp, x9

    ret


// int64_t coroutine_switch(
//      RegState *from,
//      RegState *to,
//      int64_t ret
// )
.global coroutine_switch
.type coroutine_switch, %function
.hidden coroutine_switch
coroutine_switch:
    // x0 = from
    // x1 = to
    // x2 = ret

    // save current context to "from"
    stp x19, x20, [x0, #0]
    stp x21, x22, [x0, #16]
    stp x23, x24, [x0, #32]
    stp x25, x26, [x0, #48]
    stp x27, x28, [x0, #64]
    stp x29, x30, [x0, #80]
    mov x9, sp
    str x9, [x0, #96]

    // load target context from "to"
    ldp x19, x20, [x1, #0]
    ldp x21, x22, [x1, #16]
    ldp x23, x24, [x1, #32]
    ldp x25, x26, [x1, #48]
    ldp x27, x28, [x1, #64]
    ldp x29, x30, [x1, #80]
    ldr x9, [x1, #96]
    mov sp, x9

    // set return value
    mov x0, x2

    ret

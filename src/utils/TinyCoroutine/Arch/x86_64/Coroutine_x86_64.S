.text

// int64_t coroutine_start(
//      void *arg1,
//      void *arg2,
//      int64_t (*proc)(void *, void *),
//      void *new_stack,
//      RegContext *from
//  )
    .global coroutine_start
    .type coroutine_start, @function
    .hidden coroutine_start
coroutine_start:
    // rdi = arg1
    // rsi = arg2
    // rdx = proc
    // rcx  = new_stack
    // r8  = from

    // save registers to "from"
    mov %rbx, (%r8)
    mov %rsp, 8(%r8)
    mov %rbp, 16(%r8)
    mov %r12, 24(%r8)
    mov %r13, 32(%r8)
    mov %r14, 40(%r8)
    mov %r15, 48(%r8)

    // switch stack
    mov %rcx, %rsp

    // save "from"
    mov %r8, %rbp

    // call
    call *%rdx
    
    // restore "from"
    mov %rbp, %r8

    // restore registers from "from"
    mov (%r8), %rbx
    mov 8(%r8), %rsp
    mov 16(%r8), %rbp
    mov 24(%r8), %r12
    mov 32(%r8), %r13
    mov 40(%r8), %r14
    mov 48(%r8), %r15

    // return
    ret


// int64_t coroutine_switch(
//      RegContext *from,
//      RegContext *to,
//      int64_t ret
// )
    .global coroutine_switch
    .type coroutine_switch, @function
    .hidden coroutine_switch
coroutine_switch:
    // rdi = from
    // rsi = to
    // rdx = ret

    // save registers to "from"
    mov %rbx, (%rdi)
    mov %rsp, 8(%rdi)
    mov %rbp, 16(%rdi)
    mov %r12, 24(%rdi)
    mov %r13, 32(%rdi)
    mov %r14, 40(%rdi)
    mov %r15, 48(%rdi)

    // load registers from "to"
    mov (%rsi), %rbx
    mov 8(%rsi), %rsp
    mov 16(%rsi), %rbp
    mov 24(%rsi), %r12
    mov 32(%rsi), %r13
    mov 40(%rsi), %r14
    mov 48(%rsi), %r15

    // set return value
    mov %rdx, %rax
    
    // return
    ret

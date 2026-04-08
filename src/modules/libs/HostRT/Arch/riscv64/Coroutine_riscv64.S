.text

# int64_t coroutine_start(
#      void *arg1,
#      void *arg2,
#      int64_t (*proc)(void *, void *),
#      void *new_stack,
#      RegState *from
#  )
.global coroutine_start
.type coroutine_start, @function
.hidden coroutine_start
coroutine_start:
    # a0 = arg1
    # a1 = arg2
    # a2 = proc
    # a3 = new_stack
    # a4 = from

    # save callee-saved registers and context to "from"
    sd s0,   0(a4)
    sd s1,   8(a4)
    sd s2,  16(a4)
    sd s3,  24(a4)
    sd s4,  32(a4)
    sd s5,  40(a4)
    sd s6,  48(a4)
    sd s7,  56(a4)
    sd s8,  64(a4)
    sd s9,  72(a4)
    sd s10, 80(a4)
    sd s11, 88(a4)
    sd sp,  96(a4)
    sd ra, 104(a4)

    # switch stack
    mv sp, a3

    # keep from pointer across the call
    mv s0, a4

    # call proc(arg1, arg2)
    jalr ra, 0(a2)

    # restore from pointer
    mv a4, s0

    # restore callee-saved registers and stack
    ld s0,   0(a4)
    ld s1,   8(a4)
    ld s2,  16(a4)
    ld s3,  24(a4)
    ld s4,  32(a4)
    ld s5,  40(a4)
    ld s6,  48(a4)
    ld s7,  56(a4)
    ld s8,  64(a4)
    ld s9,  72(a4)
    ld s10, 80(a4)
    ld s11, 88(a4)
    ld sp,  96(a4)
    ld ra, 104(a4)

    ret


# int64_t coroutine_switch(
#      RegState *from,
#      RegState *to,
#      int64_t ret
# )
.global coroutine_switch
.type coroutine_switch, @function
.hidden coroutine_switch
coroutine_switch:
    # a0 = from
    # a1 = to
    # a2 = ret

    # save current context to "from"
    sd s0,   0(a0)
    sd s1,   8(a0)
    sd s2,  16(a0)
    sd s3,  24(a0)
    sd s4,  32(a0)
    sd s5,  40(a0)
    sd s6,  48(a0)
    sd s7,  56(a0)
    sd s8,  64(a0)
    sd s9,  72(a0)
    sd s10, 80(a0)
    sd s11, 88(a0)
    sd sp,  96(a0)
    sd ra, 104(a0)

    # load target context from "to"
    ld s0,   0(a1)
    ld s1,   8(a1)
    ld s2,  16(a1)
    ld s3,  24(a1)
    ld s4,  32(a1)
    ld s5,  40(a1)
    ld s6,  48(a1)
    ld s7,  56(a1)
    ld s8,  64(a1)
    ld s9,  72(a1)
    ld s10, 80(a1)
    ld s11, 88(a1)
    ld sp,  96(a1)
    ld ra, 104(a1)

    # set return value
    mv a0, a2

    ret

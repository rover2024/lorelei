#include "a.h"

#include <math.h>
#include <stdint.h>

static inline void qemu_magic_ud2(uint32_t op, uint32_t tag) {
    register uint32_t eax __asm__("eax") = op;
    register uint32_t ebx __asm__("ebx") = tag;

    __asm__ volatile(
        "ud2"
        : "+a"(eax), "+b"(ebx)
        :
        : "memory");
}

double sqrt_A(double x) {
    return sqrt(x);
}

double exp_A(double x) {
    return exp(x);
}

double log_A(double x) {
    return log(x);
}

double cos_A(double x) {
    return cos(x);
}

double sin_A(double x) {
    return sin(x);
}

double tan_A(double x) {
    return tan(x);
}

void consume(double x) {
    (void)x;
}

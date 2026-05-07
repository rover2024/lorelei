#include "a.h"

#include <math.h>
#include <stdint.h>

static int m_times = 1;

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
    volatile double res;
    for (int i = 0; i < m_times; i++) {
        res = sqrt(i);
    }
    return res;
}

double exp_A(double x, double y) {
    volatile double res;
    for (int i = 0; i < m_times; i++) {
        res = exp(i);
    }
    return res;
}

double log_A(double x, double y, double z) {
    volatile double res;
    for (int i = 0; i < m_times; i++) {
        res = log(i);
    }
    return res;
}

double cos_A(double x, double y, double z, double w) {
    volatile double res;
    for (int i = 0; i < m_times; i++) {
        res = cos(i);
    }
    return res;
}

double sin_A(double x, double y, double z, double w, double v) {
    volatile double res;
    for (int i = 0; i < m_times; i++) {
        res = sin(i);
    }
    return res;
}

double tan_A(double x, double y, double z, double w, double v, double u) {
    volatile double res;
    for (int i = 0; i < m_times; i++) {
        res = tan(i);
    }
    return res;
}

void consume(double x) {
    (void)x;
}

void set_times(int times) {
    m_times = times;
}

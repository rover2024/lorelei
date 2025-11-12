#ifndef TIME_H
#define TIME_H

#include <cstdint>

namespace lore {

    // Cross-platform RDTSC implementation
    static inline uint64_t rdtsc(void) {
#ifdef __x86_64__
        uint32_t lo, hi;
        __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t) hi << 32) | lo;
#elif defined(__aarch64__)
        uint64_t val;
        __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(val));
        return val;
#elif defined(__riscv)
        uint64_t cycles;
        __asm__ __volatile__("rdcycle %0" : "=r"(cycles));
        return cycles;
#else
#  error "Unsupported architecture"
#endif
    }

    extern thread_local uint64_t LoreTicks;
    extern thread_local uint64_t LoreLastTick;
    extern thread_local uint64_t LoreTotalTicks;

}

#endif // TIME_H

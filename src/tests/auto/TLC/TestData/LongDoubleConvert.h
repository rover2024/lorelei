#pragma once

// Convert between the guest's x86 80-bit extended `long double` and the host's native long double.
//
// `long double` is one of the few types whose in-memory representation differs across ISAs: it is
// the 80-bit x87 extended format on x86_64, but IEEE-754 binary128 on aarch64 and riscv64. Because
// the guest is always x86_64, a long double that crosses the boundary arrives as 80-bit bytes and
// must be decoded into the host's format (and the result re-encoded on the way back).
//
// This is a compact version of the conversion in box64 (src/emu/x87emu_private.c, LD2D / D2LD);
// that one is fully correct for infinities, NaNs and denormals, while this keeps to the common
// finite path to stay readable.

#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

namespace lore::thunk::example {

#if defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 64

    // Host long double is already the x86 80-bit extended format, so the guest bytes match.
    inline long double f80ToHost(const void *p80) {
        long double v;
        std::memcpy(&v, p80, sizeof(long double));
        return v;
    }

    inline void hostToF80(long double v, void *p80) {
        std::memcpy(p80, &v, sizeof(long double));
    }

#else

    // Host long double differs from the guest's, so decode/encode the 80-bit extended bytes:
    // a sign bit, a 15-bit exponent (bias 16383), and a 64-bit mantissa with an explicit integer
    // bit.
    inline long double f80ToHost(const void *p80) {
        const auto *bytes = static_cast<const unsigned char *>(p80);
        uint64_t mantissa;
        uint16_t signExp;
        std::memcpy(&mantissa, bytes, 8);
        std::memcpy(&signExp, bytes + 8, 2);

        int sign = (signExp >> 15) & 1;
        int exponent = signExp & 0x7fff;

        long double v;
        if (exponent == 0x7fff) {
            v = (mantissa << 1) ? std::numeric_limits<long double>::quiet_NaN()
                                : std::numeric_limits<long double>::infinity();
        } else if (exponent == 0 && mantissa == 0) {
            v = 0.0L;
        } else {
            // value = mantissa * 2^(exponent - 16383 - 63)
            v = std::ldexp((long double) mantissa, exponent - 16383 - 63);
        }
        return sign ? -v : v;
    }

    inline void hostToF80(long double v, void *p80) {
        auto *bytes = static_cast<unsigned char *>(p80);
        uint16_t signExp = 0;
        uint64_t mantissa = 0;
        int sign = std::signbit(v) ? 1 : 0;
        v = std::fabs(v);

        if (v == 0.0L) {
            signExp = 0;
            mantissa = 0;
        } else if (std::isinf(v)) {
            signExp = 0x7fff;
            mantissa = 0x8000000000000000ULL;
        } else if (std::isnan(v)) {
            signExp = 0x7fff;
            mantissa = 0xC000000000000000ULL;
        } else {
            int exponent;
            long double m = std::frexp(v, &exponent); // v = m * 2^exponent, m in [0.5, 1)
            mantissa = (uint64_t) std::ldexp(m, 64);   // m * 2^64, integer bit set
            signExp = (uint16_t) (exponent - 1 + 16383);
        }
        signExp |= (uint16_t) (sign << 15);
        std::memcpy(bytes, &mantissa, 8);
        std::memcpy(bytes + 8, &signExp, 2);
    }

#endif

}

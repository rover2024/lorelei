#ifndef MPALAND_PRINTF_H
#define MPALAND_PRINTF_H

// Port from:
// https://github.com/mpaland/printf/blob/d3b984684bb8a8bdc48cc7a1abecb93ce59bbe3e/printf.c

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// output function type
typedef void (*out_fct_type)(char character, void *buffer, size_t idx, size_t maxlen);

enum printf_arg_type {
    pa_char = 1,
    pa_short,
    pa_int,
    pa_long,
    pa_longlong,
    pa_float,
    pa_double,
    pa_pointer,
};

struct printf_arg_entry {
    int type;
    union {
        char c;
        short s;
        int i;
        long l;
        long long ll;
        float f;
        double d;
        void *p;
    };
};


// internal buffer output
static inline void _out_buffer(char character, void *buffer, size_t idx, size_t maxlen) {
    if (idx < maxlen) {
        ((char *) buffer)[idx] = character;
    }
}

// internal null output
static inline void _out_null(char character, void *buffer, size_t idx, size_t maxlen) {
    (void) character;
    (void) buffer;
    (void) idx;
    (void) maxlen;
}

// internal _putchar wrapper
// static inline void _out_char(char character, void *buffer, size_t idx, size_t maxlen) {
//     (void) buffer;
//     (void) idx;
//     (void) maxlen;
//     if (character) {
//         _putchar(character);
//     }
// }

int _vsnprintf(out_fct_type out, char *buffer, const size_t maxlen, const char *format, va_list va,
               struct printf_arg_entry *entries);

int _vprintf(const char *format, va_list va, struct printf_arg_entry *entries);

#endif // MPALAND_PRINTF_H 
#ifndef MPALAND_PRINTF_SCANF_H
#define MPALAND_PRINTF_SCANF_H

// Port from:
// https://github.com/mpaland/printf/blob/d3b984684bb8a8bdc48cc7a1abecb93ce59bbe3e/printf.c

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "loreshared.h"

#ifdef __cplusplus
extern "C" {
#endif

// output function type
typedef void (*out_fct_type)(char character, void *buffer, size_t idx, size_t maxlen);


// internal buffer output
// static inline void _out_buffer(char character, void *buffer, size_t idx, size_t maxlen) {
//     if (idx < maxlen) {
//         ((char *) buffer)[idx] = character;
//     }
// }

// // internal null output
static inline void _out_null_scanf(char character, void *buffer, size_t idx, size_t maxlen) {
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

int _vsnprintf_scanf(out_fct_type out, char *buffer, const size_t maxlen, const char *format, va_list va,
                     struct LORE_VARG_ENTRY *entries);

int _vprintf_scanf(const char *format, va_list va, struct LORE_VARG_ENTRY *entries);

#ifdef __cplusplus
}
#endif

#endif // MPALAND_PRINTF_SCANF_H
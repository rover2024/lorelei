#ifndef FF_SCANF_H
#define FF_SCANF_H

// Port from:
// https://github.com/FFmpeg/FFmpeg/blob/958c46800e68809bd47d9bff5d55c99ed943ca41/libavutil/avsscanf.c

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FFFILE {
    size_t buf_size;
    unsigned char *buf;
    unsigned char *rpos, *rend;
    unsigned char *shend;
    ptrdiff_t shlim, shcnt;
    void *cookie;
    size_t (*read)(struct FFFILE *, unsigned char *, size_t);
} FFFILE;

int ff_vfscanf(FFFILE *f, const char *fmt, va_list ap, int *cnt);

int ff_vsscanf(const char *s, const char *fmt, va_list ap, int *cnt);

#ifdef __cplusplus
}
#endif

#endif // FF_SCANF_H
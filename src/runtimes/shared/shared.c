#define _GNU_SOURCE

#include <dlfcn.h>

#include "loreshared.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <avcall.h>

#include <ff_scanf/scanf.h>

#include <mpaland_printf/printf.h>
#include <mpaland_printf/printf_scanf.h>

static const char *PathGetName(const char *path) {
    const char *slashPos = strrchr(path, '/');
    if (!slashPos) {
        return path;
    }
    return slashPos + 1;
}

bool Lore_RevealLibraryPath(char *buffer, const void *addr, bool followSymlink) {
    Dl_info info;
    dladdr(addr, &info);

    if (!followSymlink) {
        strcpy(buffer, info.dli_fname);
        return true;
    }

    char path[PATH_MAX];
    if (!realpath(info.dli_fname, path)) {
        return false;
    }
    strcpy(buffer, path);
    return true;
}

void Lore_GetLibraryName(char *buffer, const char *path) {
    const char *start = PathGetName(path);
    const char *end = NULL;

    int len = strlen(path);
    for (const char *p = path + len - 3; p >= start; --p) {
        if (strncasecmp(p, ".so", 3) == 0) {
            end = p;
            break;
        }
    }
    if (!end) {
        end = path + len;
    }
    strncpy(buffer, start, end - start);
}

int Lore_ExtractPrintFArgs(const char *format, va_list ap, struct LORE_VARG_ENTRY *out) {
    char buffer[1];
    (void) _vsnprintf(_out_null, buffer, (size_t) -1, format, ap, out);

    struct LORE_VARG_ENTRY *p = out;
    while (p->type) {
        p++;
    }
    return p - out;
}

int Lore_ExtractSScanFArgs(const char *buffer, const char *format, va_list ap, struct LORE_VARG_ENTRY *out) {
    va_list ap1;
    va_copy(ap1, ap);

    int cnt;
    (void) ff_vsscanf(buffer, format, ap, &cnt);

    for (int i = 0; i < cnt; ++i) {
        void *p = va_arg(ap1, void *);
        out[i].type = LORE_VT_POINTER;
        out[i].p = p;
    }
    out[cnt].type = 0;
    out[cnt].p = NULL;
    return cnt;
}

int Lore_ExtractScanFArgs(const char *format, va_list ap, struct LORE_VARG_ENTRY *out) {
    char buffer[1];
    (void) _vsnprintf_scanf(_out_null, buffer, (size_t) -1, format, ap, out);

    struct LORE_VARG_ENTRY *p = out;
    while (p->type) {
        p++;
    }
    return p - out;
}

static inline void VAFmtCall_start_helper(av_alist *list, void *func, struct LORE_VARG_ENTRY *entry) {
    switch (entry->type) {
        case 0: {
            av_start_void(*list, func);
            break;
        }
        case LORE_VT_CHAR: {
            av_start_char(*list, func, &entry->c);
            break;
        }
        case LORE_VT_UCHAR: {
            av_start_uchar(*list, func, &entry->uc);
            break;
        }
        case LORE_VT_SHORT: {
            av_start_short(*list, func, &entry->s);
            break;
        }
        case LORE_VT_USHORT: {
            av_start_ushort(*list, func, &entry->us);
            break;
        }
        case LORE_VT_INT: {
            av_start_int(*list, func, &entry->i);
            break;
        }
        case LORE_VT_UINT: {
            av_start_uint(*list, func, &entry->u);
            break;
        }
        case LORE_VT_LONG: {
            av_start_long(*list, func, &entry->l);
            break;
        }
        case LORE_VT_ULONG: {
            av_start_ulong(*list, func, &entry->ul);
            break;
        }
        case LORE_VT_LONGLONG: {
            av_start_longlong(*list, func, &entry->ll);
            break;
        }
        case LORE_VT_ULONGLONG: {
            av_start_longlong(*list, func, &entry->ull);
            break;
        }
        case LORE_VT_FLOAT: {
            av_start_float(*list, func, &entry->f);
            break;
        }
        case LORE_VT_DOUBLE: {
            av_start_double(*list, func, &entry->d);
            break;
        }
        case LORE_VT_POINTER: {
            av_start_ptr(*list, func, void *, &entry->p);
            break;
        }
        default:
            break;
    }
}

static inline void VAFmtCall_arg_helper(av_alist *list, struct LORE_VARG_ENTRY *entry) {
    switch (entry->type) {
        case LORE_VT_CHAR: {
            av_char(*list, entry->c);
            break;
        }
        case LORE_VT_UCHAR: {
            av_uchar(*list, entry->uc);
            break;
        }
        case LORE_VT_SHORT: {
            av_short(*list, entry->s);
            break;
        }
        case LORE_VT_USHORT: {
            av_ushort(*list, entry->us);
            break;
        }
        case LORE_VT_INT: {
            av_int(*list, entry->i);
            break;
        }
        case LORE_VT_UINT: {
            av_int(*list, entry->u);
            break;
        }
        case LORE_VT_LONG: {
            av_long(*list, entry->l);
            break;
        }
        case LORE_VT_ULONG: {
            av_ulong(*list, entry->ul);
            break;
        }
        case LORE_VT_LONGLONG: {
            av_longlong(*list, entry->ll);
            break;
        }
        case LORE_VT_ULONGLONG: {
            av_ulonglong(*list, entry->ull);
            break;
        }
        case LORE_VT_FLOAT: {
            av_float(*list, entry->f);
            break;
        }
        case LORE_VT_DOUBLE: {
            av_double(*list, entry->d);
            break;
        }
        case LORE_VT_POINTER: {
            av_ptr(*list, void *, entry->p);
            break;
        }
        default:
            break;
    }
}

static inline int varg_entry_length(struct LORE_VARG_ENTRY *argv) {
    int argc = 0;
    for (struct LORE_VARG_ENTRY *p = argv; p && p->type; ++p) {
        argc++;
    }
    return argc;
}

void Lore_VAFmtCall(void *func, int argc1, struct LORE_VARG_ENTRY *argv1, int argc2, struct LORE_VARG_ENTRY *argv2,
                    struct LORE_VARG_ENTRY *ret) {
    av_alist alist;
    if (argc1 < 0) {
        argc1 = varg_entry_length(argv1);
    }
    if (argc2 < 0) {
        argc2 = varg_entry_length(argv2);
    }

    VAFmtCall_start_helper(&alist, func, ret);

    for (int i = 0; i < argc1; ++i) {
        VAFmtCall_arg_helper(&alist, argv1 + i);
    }
    for (int i = 0; i < argc2; ++i) {
        VAFmtCall_arg_helper(&alist, argv2 + i);
    }

    av_call(alist);
}

static void Lore_VAFmtCallV_Helper(void *func, int argc1, struct LORE_VARG_ENTRY *argv1, struct LORE_VARG_ENTRY *ret,
                                   ...) {

    va_list ap;
    va_start(ap, ret);
    {
        av_alist alist;
        if (argc1 < 0) {
            argc1 = varg_entry_length(argv1);
        }
        VAFmtCall_start_helper(&alist, func, ret);
        for (int i = 0; i < argc1; ++i) {
            VAFmtCall_arg_helper(&alist, argv1 + i);
        }
#ifdef __x86_64__
        av_ptr(alist, void *, ap);
#else
        av_struct(alist, va_list, ap);
#endif
        av_call(alist);
    }
    va_end(ap);
}

void Lore_VAFmtCallV(void *func, int argc1, struct LORE_VARG_ENTRY *argv1, int argc2, struct LORE_VARG_ENTRY *argv2,
                     struct LORE_VARG_ENTRY *ret) {
    av_alist alist;
    if (argc2 < 0) {
        argc2 = varg_entry_length(argv2);
    }

    av_start_void(alist, Lore_VAFmtCallV_Helper);
    av_ptr(alist, void *, func);
    av_int(alist, argc1);
    av_ptr(alist, void *, argv1);
    av_ptr(alist, void *, ret);

    for (int i = 0; i < argc2; ++i) {
        VAFmtCall_arg_helper(&alist, argv2 + i);
    }

    av_call(alist);
}

// v: void
// c: char
// C: unsigned char
// w: short
// W: unsigned short
// i: int
// u: unsigned int
// I: long
// U: unsigned long
// l: long long
// L: unsigned long long
// p: pointer

static void AVCall_start_helper(av_alist *list, void *func, void *ret, char c) {
    switch (c) {
        case 'c': {
            av_start_char(*list, func, ret);
            break;
        }
        case 'C': {
            av_start_uchar(*list, func, ret);
            break;
        }
        case 'w': {
            av_start_short(*list, func, ret);
            break;
        }
        case 'W': {
            av_start_ushort(*list, func, ret);
            break;
        }
        case 'i': {
            av_start_int(*list, func, ret);
            break;
        }
        case 'u': {
            av_start_uint(*list, func, ret);
            break;
        }
        case 'I': {
            av_start_long(*list, func, ret);
            break;
        }
        case 'U': {
            av_start_ulong(*list, func, ret);
            break;
        }
        case 'l': {
            av_start_longlong(*list, func, ret);
            break;
        }
        case 'L': {
            av_start_ulonglong(*list, func, ret);
            break;
        }
        case 'p': {
            av_start_ptr(*list, func, void *, ret);
            break;
        }
    }
}

static void AVCall_arg_helper(av_alist *list, void *arg, char c) {
    switch (c) {
        case 'c': {
            av_char(*list, *(char *) arg);
            break;
        }
        case 'C': {
            av_uchar(*list, *(unsigned char *) arg);
            break;
        }
        case 'w': {
            av_short(*list, *(short *) arg);
            break;
        }
        case 'W': {
            av_ushort(*list, *(unsigned short *) arg);
            break;
        }
        case 'i': {
            av_int(*list, *(int *) arg);
            break;
        }
        case 'u': {
            av_uint(*list, *(unsigned int *) arg);
            break;
        }
        case 'I': {
            av_long(*list, *(long *) arg);
            break;
        }
        case 'U': {
            av_ulong(*list, *(unsigned long *) arg);
            break;
        }
        case 'l': {
            av_longlong(*list, *(long long *) arg);
            break;
        }
        case 'L': {
            av_ulonglong(*list, *(unsigned long long *) arg);
            break;
        }
        case 'p': {
            av_ptr(*list, void *, *(void **) arg);
            break;
        }
    }
}

void Lore_AVCall(void *func, void **args, void *ret, const char *convention) {
    if (convention[1] != 'F') {
        return;
    }

    av_alist alist;
    AVCall_start_helper(&alist, func, ret, convention[0]);

    const char *p = &convention[2];
    void **arg = args;
    for (; *p; ++p, ++arg) {
        AVCall_arg_helper(&alist, arg, *p);
    }
}
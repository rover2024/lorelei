#include "loreshared.h"

#define _GNU_SOURCE

#include <dlfcn.h>

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <avcall.h>

#include <ff_scanf/scanf.h>
#include <mpaland_printf/printf.h>

static const char *PathGetName(const char *path) {
    const char *slashPos = strrchr(path, '/');
    if (!slashPos) {
        return path;
    }
    return slashPos + 1;
}

bool Lore_RevealLibraryPath(char *buffer, const void *addr) {
    Dl_info info;
    dladdr(addr, &info);

    char path[PATH_MAX];
    if (!realpath(info.dli_fname, path)) {
        return false;
    }
    strcpy(buffer, PathGetName(path));
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
    (void) _vsnprintf(_out_null, buffer, (size_t) -1, format, ap, (struct printf_arg_entry *) out);

    struct LORE_VARG_ENTRY *p = out;
    while (p->type) {
        p++;
    }
    return p - out;
}

int Lore_ExtractSScanFArgs(const char *buffer, const char *format, va_list ap, void **out) {
    va_list ap1;
    va_copy(ap1, ap);

    int cnt;
    (void) ff_vsscanf(buffer, format, ap, &cnt);

    for (int i = 0; i < cnt; ++i) {
        void *p = va_arg(ap1, void *);
        out[i] = p;
    }
    out[cnt] = NULL;
    return cnt;
}

void Lore_VariadicCall(void *func, int argc, struct LORE_VARG_ENTRY *argv, struct LORE_VARG_ENTRY *ret) {
    av_alist alist;

    // Add return value
    switch (ret->type) {
        case 0: {
            av_start_void(alist, func);
            break;
        }
        case LORE_VT_CHAR: {
            av_start_char(alist, func, &ret->c);
            break;
        }
        case LORE_VT_SHORT: {
            av_start_short(alist, func, &ret->s);
            break;
        }
        case LORE_VT_INT: {
            av_start_int(alist, func, &ret->i);
            break;
        }
        case LORE_VT_LONG: {
            av_start_long(alist, func, &ret->l);
            break;
        }
        case LORE_VT_LONGLONG: {
            av_start_longlong(alist, func, &ret->ll);
            break;
        }
        case LORE_VT_FLOAT: {
            av_start_float(alist, func, &ret->f);
            break;
        }
        case LORE_VT_DOUBLE: {
            av_start_double(alist, func, &ret->d);
            break;
        }
        case LORE_VT_POINTER: {
            av_start_ptr(alist, func, void *, &ret->p);
            break;
        }
        default:
            break;
    }

    // Add arguments
    for (int i = 0; i < argc; ++i) {
        struct LORE_VARG_ENTRY *p = argv + i;
        switch (p->type) {
            case LORE_VT_CHAR: {
                av_char(alist, p->c);
                break;
            }
            case LORE_VT_SHORT: {
                av_short(alist, p->s);
                break;
            }
            case LORE_VT_INT: {
                av_int(alist, p->i);
                break;
            }
            case LORE_VT_LONG: {
                av_long(alist, p->l);
                break;
            }
            case LORE_VT_LONGLONG: {
                av_longlong(alist, p->ll);
                break;
            }
            case LORE_VT_FLOAT: {
                av_float(alist, p->f);
                break;
            }
            case LORE_VT_DOUBLE: {
                av_double(alist, p->d);
                break;
            }
            case LORE_VT_POINTER: {
                av_ptr(alist, void *, p->p);
                break;
            }
            default:
                break;
        }
    }

    // Call
    av_call(alist);
}
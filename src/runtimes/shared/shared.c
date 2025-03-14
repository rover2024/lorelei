#include "loreshared.h"

#define _GNU_SOURCE

#include <dlfcn.h>

#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <ff_scanf/scanf.h>
#include <mpaland_printf/printf.h>

bool Lore_RevealLibraryPath(char *buffer, const void *addr) {
    Dl_info info;
    dladdr(addr, &info);

    char path[PATH_MAX];
    if (!realpath(info.dli_fname, path)) {
        return false;
    }

    strcpy(buffer, strrchr(buffer, '/') + 1);
    return true;
}

void Lore_ExtractPrintFArgs(const char *format, va_list ap, struct LORE_VARG_ENTRY *out) {
    char buffer[1];
    (void) _vsnprintf(_out_null, buffer, (size_t) -1, format, ap, (struct printf_arg_entry *) out);
}

void Lore_ExtractSScanFArgs(const char *buffer, const char *format, va_list ap, void **out) {
    va_list ap1;
    va_copy(ap1, ap);

    int cnt;
    (void) ff_vsscanf(buffer, format, ap, &cnt);

    for (int i = 0; i < cnt; ++i) {
        void *p = va_arg(ap1, void *);
        out[i] = p;
    }
    out[cnt] = NULL;
}
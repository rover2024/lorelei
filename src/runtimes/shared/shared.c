#define _GNU_SOURCE

#include <dlfcn.h>

#include "loreshared.h"

#include <string.h>
#include <stdlib.h>
#include <limits.h>

static const char *PathGetName(const char *path) {
    const char *slashPos = strrchr(path, '/');
    if (!slashPos) {
        return path;
    }
    return slashPos + 1;
}

int Lore_RevealLibraryPath(char *buffer, const void *addr, int followSymlink) {
    Dl_info info;
    dladdr(addr, &info);

    if (!followSymlink) {
        strcpy(buffer, info.dli_fname);
        return 1;
    }

    char path[PATH_MAX];
    if (!realpath(info.dli_fname, path)) {
        return 0;
    }
    strcpy(buffer, path);
    return 1;
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
    memcpy(buffer, start, end - start);
    buffer[end - start] = '\0';
}
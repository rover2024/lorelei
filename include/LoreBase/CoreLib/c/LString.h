#ifndef LORE_BASE_CORELIB_LSTRING_H
#define LORE_BASE_CORELIB_LSTRING_H

#include <stddef.h>
#include <stdbool.h>

#include <LoreBase/CoreLib/Global.h>

LORE_BEGIN_EXTERN_C

struct LString {
    char *data;
    size_t size;
};

struct LStringList {
    struct LString *data;
    size_t size;
};

LORE_END_EXTERN_C

#endif // LORE_BASE_CORELIB_LSTRING_H

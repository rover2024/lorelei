#ifndef LORE_BASE_PASSTHROUGH_CSTRING_H
#define LORE_BASE_PASSTHROUGH_CSTRING_H

#include <stddef.h>
#include <stdbool.h>

#include <lorelei/Base/Support/Global.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CString {
    char *data;
    size_t size;
};

struct CStringList {
    struct CString *data;
    size_t size;
};

#ifdef __cplusplus
}
#endif

#endif // LORE_BASE_PASSTHROUGH_CSTRING_H

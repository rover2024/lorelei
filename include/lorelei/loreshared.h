#ifndef LORESHARED_H
#define LORESHARED_H

#include <stdarg.h>
#include <stdbool.h>

#include <lorelei/lorelei_global.h>

#ifdef __cplusplus
extern "C" {
#endif

struct LORE_GUEST_LIBRARY_DATA {
    const char *name;
    const char *path;
    const char *hostThunk;
    const char *host;

    int dependencyCount;
    const char **dependencies;
};

struct LORE_HOST_LIBRARY_DATA {
    const char *name;
    const char *fileName;

    int guestThunkCount;
    const char **guestThunks;
};

//
// Path APIs
//
LORELEI_EXPORT bool Lore_RevealLibraryPath(char *buffer, const void *addr);

LORELEI_EXPORT void Lore_GetLibraryName(char *buffer, const char *path);


//
// Variadic argument APIs.
//
enum LORE_VARG_TYPE {
    LORE_VT_CHAR = 1,
    LORE_VT_SHORT,
    LORE_VT_INT,
    LORE_VT_LONG,
    LORE_VT_LONGLONG,
    LORE_VT_FLOAT,
    LORE_VT_DOUBLE,
    LORE_VT_POINTER,
};

struct LORE_VARG_ENTRY {
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

LORELEI_EXPORT int Lore_ExtractPrintFArgs(const char *format, va_list ap, struct LORE_VARG_ENTRY *out);

LORELEI_EXPORT int Lore_ExtractSScanFArgs(const char *buffer, const char *format, va_list ap, void **out);

LORELEI_EXPORT void Lore_VariadicCall(void *func, int argc, struct LORE_VARG_ENTRY *argv, struct LORE_VARG_ENTRY *ret);

#ifdef __cplusplus
}
#endif

#endif // LORESHARED_H
#ifndef LOREHAPI_H
#define LOREHAPI_H

#include <stdbool.h>

#include <lorelei/lorelei_global.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LORELEI_HOST_LIBRARY

//
// Host library APIs
//
LORELEI_EXPORT void *Lore_GetFPExecuteCallback();

LORELEI_EXPORT void *Lore_GetCallbackThunk(const char *sign);


//
// Host thunk APIs
//
LORELEI_EXPORT void *Lore_GetLibraryDataImpl(const char *path, bool isGuest);


//
// Guest thunk utility APIs
//
LORELEI_EXPORT void *Lore_LoadHostLibrary(void *someAddr);

LORELEI_EXPORT void Lore_FreeHostLibrary(void *handle);

#endif

#ifdef __cplusplus
}
#endif

#endif // LOREHAPI_H
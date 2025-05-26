#ifndef LOREHAPI_H
#define LOREHAPI_H

#include <lorelei/lorelei_global.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LORELEI_HOST_LIBRARY

struct LoreEmuApis;

//
// Host thunk APIs
//
LORELEI_EXPORT struct LoreEmuApis *Lore_HrtGetEmuApis();

LORELEI_EXPORT void Lore_HrtSetThreadCallback(void *callback);

LORELEI_EXPORT void *Lore_HrtGetLibraryData(const char *path, int isThunk);


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
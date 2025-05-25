#ifndef LOREHAPI_H
#define LOREHAPI_H

#include <lorelei/lorelei_global.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LORELEI_HOST_LIBRARY

struct LoreEmuApis;

//
// Host library APIs
//
LORELEI_EXPORT struct LoreEmuApis *Lore_HrtGetEmuApis();

LORELEI_EXPORT void Lore_HrtSetThreadCallback(void *callback);


//
// Host thunk APIs
//
LORELEI_EXPORT void *Lore_HrtGetLibraryData(const char *path, int isThunk);

LORELEI_EXPORT void *Lore_HrtGetLibraryThunks(const char *path, int isGuest);


//
// Guest thunk utility APIs
//
LORELEI_EXPORT void *Lore_LoadHostLibrary(void *someAddr, int thunkCount, void **thunks);

LORELEI_EXPORT void Lore_FreeHostLibrary(void *handle);

#endif

#ifdef __cplusplus
}
#endif

#endif // LOREHAPI_H
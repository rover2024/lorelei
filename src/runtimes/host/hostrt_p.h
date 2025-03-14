#ifndef HOSTRT_P_H
#define HOSTRT_P_H

#include <pthread.h>

#include <lorelei/lorelei_global.h>

typedef void      (*FP_ExecuteCallback)(void * /*thunk*/, void * /*callback*/, void * /*args*/, void * /*ret*/);
typedef pthread_t (*FP_GetLastPThreadId)(void);
typedef void      (*FP_NotifyPThreadCreate)(pthread_t * /*thread*/, const pthread_attr_t * /*attr*/, void *(*) (void *) /*start_routine*/, void * /*arg*/, int * /*ret*/);
typedef void      (*FP_NotifyPThreadExit)(void * /*ret*/);

struct LoreEmuApis {
    // Executes guest callback
    FP_ExecuteCallback ExecuteCallback;

    // Get last pthread id
    FP_GetLastPThreadId GetLastPThreadId;

    // Notify emulator to create thread in guest environment
    FP_NotifyPThreadCreate NotifyPThreadCreate;

    // Notify emulator to exit thread in guest environment
    FP_NotifyPThreadExit NotifyPThreadExit;
};

#ifdef __cplusplus
extern "C" {
#endif

LORELEI_DECL_EXPORT struct LoreEmuApis *Lore_HRTGetEmuApis();

LORELEI_DECL_EXPORT const char *Lore_HRTGetValue(const char *key);

LORELEI_DECL_EXPORT bool Lore_HRTSetValue(const char *key, const char *value);

#ifdef __cplusplus
}
#endif

#endif // HOSTRT_P_H
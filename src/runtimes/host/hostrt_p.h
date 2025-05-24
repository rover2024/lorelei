#ifndef HOSTRT_P_H
#define HOSTRT_P_H

#include <pthread.h>
#include <stdbool.h>

#include <lorelei/lorelei_global.h>

typedef void      (*FP_ExecuteCallback)(void * /*thunk*/, void * /*callback*/, void * /*args*/, void * /*ret*/, void * /*metadata*/);
typedef pthread_t (*FP_GetLastPThreadId)(void);
typedef void      (*FP_NotifyPThreadCreate)(pthread_t * /*thread*/, const pthread_attr_t * /*attr*/, void *(*) (void *) /*start_routine*/, void * /*arg*/, int * /*ret*/);
typedef void      (*FP_NotifyPThreadExit)(void * /*ret*/);
typedef void      (*FP_NotifyHostLibraryOpen)(const char * /*identifier*/);

struct LoreEmuApis {
    // Executes guest callback
    FP_ExecuteCallback ExecuteCallback;

    // Get last pthread id
    FP_GetLastPThreadId GetLastPThreadId;

    // Notify emulator to create thread in guest environment
    FP_NotifyPThreadCreate NotifyPThreadCreate;

    // Notify emulator to exit thread in guest environment
    FP_NotifyPThreadExit NotifyPThreadExit;

    // Notify guest runtime to load GTL
    FP_NotifyHostLibraryOpen NotifyHostLibraryOpen;
};

#ifdef __cplusplus
extern "C" {
#endif

//
// Emulator APIs
//
LORELEI_DECL_EXPORT void Lore_HandleExtraGuestCall(int type, void **args, void *ret);

LORELEI_DECL_EXPORT void Lore_HostHelper(int id, void **args, void *ret);

#ifdef __cplusplus
}
#endif

#endif // HOSTRT_P_H
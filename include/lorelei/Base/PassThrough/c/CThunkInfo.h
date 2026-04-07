#ifndef LORE_BASE_PASSTROUGH_CTHUNKINFO_H
#define LORE_BASE_PASSTROUGH_CTHUNKINFO_H

#include <lorelei/Base/PassThrough/c/CString.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CForwardThunkInfo {
    char *name;
    struct CStringList alias;
    char *guestThunk;
    char *hostThunk;
    char *hostLibrary;
};

struct CReversedThunkInfo {
    char *name;
    struct CStringList alias;
    char *fileName;
    struct CStringList thunks;
};

union CThunkInfo {
    struct CForwardThunkInfo *forward;
    struct CReversedThunkInfo *reversed;
};

#ifdef __cplusplus
}
#endif

#endif // LORE_BASE_PASSTROUGH_CTHUNKINFO_H

#ifndef THUNKINFOCDEFS_H
#define THUNKINFOCDEFS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CForwardThunkInfo {
    char *name;
    size_t numAlias;
    char **aliases;
    char *guestThunk;
    char *hostThunk;
    char *hostLibrary;
};

struct CReversedThunkInfo {
    char *name;
    size_t numAlias;
    char **aliases;
    char *fileName;
    size_t numThunks;
    char **thunks;
};

struct CThunkInfo {
    bool hasForward;
    CForwardThunkInfo forward;
    bool hasReversed;
    CReversedThunkInfo reversed;
};

#ifdef __cplusplus
}
#endif

#endif // THUNKINFOCDEFS_H

#ifndef LORE_BASE_RUNTIMEBASE_LTHUNKINFO_H
#define LORE_BASE_RUNTIMEBASE_LTHUNKINFO_H

#include <LoreBase/CoreLib/c/LString.h>

LORE_BEGIN_EXTERN_C

struct LForwardThunkInfo {
    char *name;
    struct LStringList alias;
    char *guestThunk;
    char *hostThunk;
    char *hostLibrary;
};

struct LReversedThunkInfo {
    char *name;
    struct LStringList alias;
    char *fileName;
    struct LStringList thunks;
};

union LThunkInfo {
    struct LForwardThunkInfo *forward;
    struct LReversedThunkInfo *reversed;
};

LORE_END_EXTERN_C

#endif // LORE_BASE_RUNTIMEBASE_LTHUNKINFO_H

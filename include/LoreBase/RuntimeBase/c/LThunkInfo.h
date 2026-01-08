#ifndef LORE_BASE_RUNTIMEBASE_LTHUNKINFO_H
#define LORE_BASE_RUNTIMEBASE_LTHUNKINFO_H

#include <LoreBase/CoreLib/c/LVectorIterator.h>

LORE_BEGIN_EXTERN_C

struct LForwardThunkInfo {
    char *name;
    struct LVectorIterator alias;
    char *guestThunk;
    char *hostThunk;
    char *hostLibrary;
};

struct LReversedThunkInfo {
    char *name;
    struct LVectorIterator alias;
    char *fileName;
    struct LVectorIterator thunks;
};

union LThunkInfo {
    LForwardThunkInfo *forward;
    LReversedThunkInfo *reversed;
};

LORE_END_EXTERN_C

#endif // LORE_BASE_RUNTIMEBASE_LTHUNKINFO_H

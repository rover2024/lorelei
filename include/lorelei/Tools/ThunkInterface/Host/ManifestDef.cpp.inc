#define LORE_THUNK_HOST

#include <lorelei/Tools/ThunkInterface/Callback.h>

#ifdef LORE_THUNK_BUILD
#  include "ProcDefOnBuild.cpp.inc"
#else
#  include "ProcDefPreBuild.cpp.inc"
#endif

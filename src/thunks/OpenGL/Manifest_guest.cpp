#include "Desc.h"
#include <lorelei/Tools/ThunkInterface/Guest/ManifestDef.cpp.inc>

namespace lore::thunk {

    template <>
    struct ProcFn<::glDebugMessageCallback, GuestToHost, Entry> {
        static void invoke(GLDEBUGPROC callback, const void *userParam) {
        }
    };

}

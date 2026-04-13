#include "Desc.h"
#include <lorelei/Tools/ThunkInterface/Guest/ManifestDef.cpp.inc>

#include "GL_ProcFilters_guest.h"

namespace lore::thunk {

    template <>
    struct ProcFn<::glDebugMessageCallback, GuestToHost, Entry> {
        static void invoke(GLDEBUGPROC callback, const void *userParam) {
        }
    };

    template <>
    struct ProcFn<::glDebugMessageCallbackAMD, GuestToHost, Entry> {
        static void invoke(GLDEBUGPROCAMD callback, void *userParam) {
        }
    };

    template <>
    struct ProcFn<::glDebugMessageCallbackARB, GuestToHost, Entry> {
        static void invoke(GLDEBUGPROCARB callback, const void *userParam) {
        }
    };

}

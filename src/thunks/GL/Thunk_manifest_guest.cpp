#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

// #define LORETHUNK_CALLBACK_REPLACE

#include "GL_procFilters_guest.h"

namespace lorethunk {

    template <>
    struct MetaProc<::glDebugMessageCallback, _HFN, _GTP> {
        _PROC void invoke(GLDEBUGPROC callback, const void *userParam) {
        }
    };

    template <>
    struct MetaProc<::glDebugMessageCallbackAMD, _HFN, _GTP> {
        _PROC void invoke(GLDEBUGPROCAMD callback, void *userParam) {
        }
    };

    template <>
    struct MetaProc<::glDebugMessageCallbackARB, _HFN, _GTP> {
        _PROC void invoke(GLDEBUGPROCARB callback, const void *userParam) {
        }
    };

}
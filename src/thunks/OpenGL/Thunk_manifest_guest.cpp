#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

#include <dlfcn.h>

// #define LORETHUNK_CALLBACK_REPLACE

namespace lorethunk {

    template <>
    struct MetaProc<::glDebugMessageCallback, _HFN, _GTP> {
        _PROC void invoke(GLDEBUGPROC callback, const void *userParam) {
        }
    };

}
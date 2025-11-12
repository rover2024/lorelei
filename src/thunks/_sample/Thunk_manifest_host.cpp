#include <cstring>

// Include the function description header
#include "Thunk_procs_desc.h"

// Include the common manifest context header
#include <lorelei/TLCMeta/ManifestContext_host.inc.h>

namespace lorethunk {

    // Add your custom thunks in this namespace

    template <>
    struct MetaProc<::memcpy, _HFN, _HTP> {
        _PROC void invoke(void **args, void *ret, void *metadata) {
            *(void **) ret = nullptr;
        }
    };

}
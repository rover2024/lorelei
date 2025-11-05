#include <cstdio>

// Include the function description header
#include "Thunk_procs_desc.h"

// Include the common manifest context header
#include <lorelei/TLCMeta/ManifestContext_host.inc.h>

namespace lorethunk {

    // Add your custom thunks in this namespace

    template <>
    struct MetaProc<::printf, _HFN, _HTP> {
        using HTP_IMPL = MetaProc<::printf, _HFN, _HTP_IMPL>;

        _PROC void invoke(void *args[], void *ret, void *metadata) {
            HTP_IMPL::invoke(nullptr, 1, 2);
        }
    };

}
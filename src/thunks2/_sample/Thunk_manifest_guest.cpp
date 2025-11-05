#include <cstring>

// Include the function description header
#include "Thunk_procs_desc.h"

// Include the common manifest context header
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

namespace lorethunk {

    // Add your custom thunks in this namespace

    template <>
    struct MetaProc<::memcpy, _HFN, _GTP> {
        using GTP_IMPL = MetaProc<::memcpy, _HFN, _GTP_IMPL>;

        _PROC void *invoke(void *dest, const void *src, size_t n) {
            return GTP_IMPL::invoke(dest, src, n);
        }
    };

    template <>
    struct MetaProcCB<PFN_memcpy, _GCB, _GTP> {
        using GTP_IMPL = MetaProcCB<PFN_memcpy, _GCB, _GTP_IMPL>;

        _PROC void *invoke(void *callback, void *dest, const void *src, size_t n) {
            return GTP_IMPL::invoke(callback, dest, src, n);
        }
    };

}
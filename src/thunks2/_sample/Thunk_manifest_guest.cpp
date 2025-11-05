#include <cstdio>

// Include the function description header
#include "Thunk_procs_desc.h"

// Include the common manifest context header
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

namespace lorethunk {

    // Add your custom thunks in this namespace

    template <>
    struct MetaProc<::printf, _HFN, _GTP> {
        using GTP_IMPL = MetaProc<::printf, _HFN, _GTP_IMPL>;

        _PROC int invoke(const char *fmt, ...) {
            return GTP_IMPL::invoke(fmt);
        }
    };

    using PFN_printf = lore::remove_attr_t<printf>;

    template <>
    struct MetaProcCB<PFN_printf, _GCB, _GTP> {
        using GTP_IMPL = MetaProcCB<PFN_printf, _GCB, _GTP_IMPL>;

        _PROC int invoke(void *callback, const char *fmt, ...) {
            return GTP_IMPL::invoke(callback, fmt);
        }
    };

}
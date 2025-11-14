#pragma once

#include <zstd.h>
#include <zstd_errors.h>
#include <zdict.h>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_User> {
        _DESC char moduleName[] = "libzstd";
    };

}

#ifdef gzgetc
#  undef gzgetc
#endif
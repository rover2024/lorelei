#pragma once

#include <cstring>
#include <cstdio>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    /// STEP: Add your configurations
    template <>
    struct MetaConfig<MCS_User> {
        _DESC char moduleName[] = "sample"; // required
    };

    /// STEP: Add your function descriptions
    template <>
    struct MetaProcDesc<printf> {
        using overlay_type = void (*)(const char *, void *); // overlay type
    };

    template <>
    struct MetaProcDesc<scanf> {
        _DESC MetaPass_scanf<> builder_pass = {};
    };

    /// STEP: Add your callback descriptions
    using PFN_printf = int (*)(const char *, ...);
    using PFN_memcpy = void *(*) (void *, const void *, size_t) noexcept;

    template <>
    struct MetaProcCBDesc<PFN_printf> {
        _DESC char name[] = "PFN_printf";
        _DESC MetaPass_printf<> builder_pass = {};
    };

}
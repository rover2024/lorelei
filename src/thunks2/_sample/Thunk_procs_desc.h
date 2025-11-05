#include <cstdio>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    /// STEP: Add your custom configurations
    template <>
    struct MetaConfig<MCS_User> {
        static constexpr const char moduleName[] = "sample"; // required
    };

    /// STEP: Add your custom function descriptions

    // Functions
    template <>
    struct MetaProcDesc<printf> {
        using builder_pass = MetaPass_printf<>;              // builder pass
        using overlay_type = void (*)(const char *, void *); // overlay type
    };

    template <>
    struct MetaProcDesc<scanf> {
        using builder_pass = MetaPass_scanf<>;
    };

    // Callbacks
    using PFN_printf = int (*)(const char *, ...);

    template <>
    struct MetaProcCBDesc<PFN_printf> {
        using builder_pass = MetaPass_printf<>;                           // builder pass
        using passes = MetaPassList<MetaPass_printf<>, MetaPass_scanf<>>; // other passes
        static constexpr const char name[] = "printf";
    };

}
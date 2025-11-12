#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

// #define LORETHUNK_CALLBACK_REPLACE

namespace lorethunk {

    template <>
    struct MetaProcArgFilter<::Display *> {
        template <class MetaDesc, size_t Index, class... Args>
        static void filter(::Display *&display, MetaProcArgContext<Args...> ctx) {
            XSync(display, false);
        }
    };

    template <>
    struct MetaProcReturnFilter<::XVisualInfo *> {
        template <class MetaDesc, class... Args>
        static void filter(::XVisualInfo *&visualInfo, MetaProcArgContext<Args...> ctx) {
            Display *display = ctx.template type<::Display *>();
            int nitems_return = 0;
            visualInfo = XGetVisualInfo(display, VisualScreenMask | VisualIDMask, visualInfo,
                                        &nitems_return);
            assert(nitems_return == 1);
            assert(visualInfo);
        }
    };

}
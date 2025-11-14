#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_host.inc.h>

// #define LORETHUNK_CALLBACK_REPLACE

#include <lorelei/Midware/Host/X11/Lore_X11.h>

namespace lorethunk {

    template <>
    struct MetaProcArgFilter<::Display *> {
        template <class MetaDesc, size_t Index, class... Args>
        static void filter(::Display *&display, MetaProcArgContext<Args...> ctx) {
            display = lore::midware::host::X11::Display_G2H(display);
        }
    };

    template <>
    struct MetaProcReturnFilter<::Display *> {
        template <class MetaDesc, class... Args>
        static void filter(::Display *&display, MetaProcArgContext<Args...> ctx) {
            display = lore::midware::host::X11::Display_H2G(display);
        }
    };

    template <>
    struct MetaProcArgFilter<::XVisualInfo *> {
        template <class MetaDesc, size_t Index, class... Args>
        static void filter(::XVisualInfo *&visualInfo, MetaProcArgContext<Args...> ctx) {
            Display *display = ctx.template type<::Display *>();
            visualInfo = lore::midware::host::X11::VisualInfo_G2H(display, visualInfo);
        }
    };

    template <>
    struct MetaProcReturnFilter<::XVisualInfo *> {
        template <class MetaDesc, class... Args>
        static void filter(::XVisualInfo *&visualInfo, MetaProcArgContext<Args...> ctx) {
            Display *display = ctx.template type<::Display *>();
            visualInfo = lore::midware::host::X11::VisualInfo_H2G(display, visualInfo);
        }
    };

}
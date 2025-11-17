#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <cassert>

#include <lorelei/TLCMeta/MetaProc.h>
#include <lorelei/TLCMeta/ManifestlGlobal.h>

namespace lorethunk {

    template <>
    struct MetaProcArgFilter<::Display *> {
        template <class MetaDesc, size_t Index, CProcKind ProcKind, class... Args>
        static void filter(::Display *&display, MetaProcArgContext<Args...> ctx) {
            if constexpr (CProcKind_isHost(ProcKind)) {
                XSync(display, false);
            }
        }
    };

    template <>
    struct MetaProcReturnFilter<::XVisualInfo *> {
        template <class MetaDesc, CProcKind ProcKind, class... Args>
        static void filter(::XVisualInfo *&visualInfo, MetaProcArgContext<Args...> ctx) {
            if constexpr (CProcKind_isHost(ProcKind)) {
                Display *display = ctx.template type<::Display *>();
                int nitems_return = 0;
                visualInfo = XGetVisualInfo(display, VisualScreenMask | VisualIDMask, visualInfo,
                                            &nitems_return);
                assert(nitems_return == 1);
                assert(visualInfo);
            }
        }
    };

}
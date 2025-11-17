#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <cassert>

#include <lorelei/TLCMeta/MetaProc.h>
#include <lorelei/TLCMeta/ManifestlGlobal.h>

#include <lorelei/Midware/Host/X11/Lore_X11.h>

extern "C" {
Display *GTL_XOpenDisplay(const char *);
int GTL_XSync(Display *, int);
}

namespace lorethunk {

    template <>
    struct MetaProcArgFilter<::Display *> {
        template <class MetaDesc, size_t Index, CProcKind ProcKind, class... Args>
        static void filter(::Display *&display, MetaProcArgContext<Args...> ctx) {
            if constexpr (CProcKind_isHost(ProcKind)) {
                display = lore::midware::host::X11::Display_G2H(display);
            }
        }
    };

    template <>
    struct MetaProcReturnFilter<::Display *> {
        template <class MetaDesc, CProcKind ProcKind, class... Args>
        static void filter(::Display *&display, MetaProcArgContext<Args...> ctx) {
            if constexpr (CProcKind_isHost(ProcKind)) {
                display = lore::midware::host::X11::Display_H2G(display, GTL_XOpenDisplay);
            }
        }
    };

    template <>
    struct MetaProcArgFilter<::XVisualInfo *> {
        template <class MetaDesc, size_t Index, CProcKind ProcKind, class... Args>
        static void filter(::XVisualInfo *&visualInfo, MetaProcArgContext<Args...> ctx) {
            if constexpr (CProcKind_isHost(ProcKind)) {
                Display *display = ctx.template type<::Display *>();
                visualInfo = lore::midware::host::X11::VisualInfo_G2H(display, visualInfo);
            }
        }
    };

    template <>
    struct MetaProcReturnFilter<::XVisualInfo *> {
        template <class MetaDesc, CProcKind ProcKind, class... Args>
        static void filter(::XVisualInfo *&visualInfo, MetaProcArgContext<Args...> ctx) {
            if constexpr (CProcKind_isHost(ProcKind)) {
                Display *display = ctx.template type<::Display *>();
                visualInfo = lore::midware::host::X11::VisualInfo_H2G(display, visualInfo);
            }
        }
    };

}
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <cassert>

#include <lorelei/Modules/Midware/Host/X11/Lore_X11.h>
#include <lorelei/Tools/ThunkInterface/Proc.h>

extern "C" {
Display *GTL_XOpenDisplay(const char *);
int GTL_XSync(Display *, int);
}

namespace lore::thunk {

    // template <>
    // struct ProcArgFilter<::Display *> {
    //     template <class Desc, size_t Index, ProcKind Kind, ProcDirection Direction, class... Args>
    //     static void filter(::Display *&display, ProcArgContext<Args...> ctx) {
    //         if constexpr (Direction == GuestToHost) {
    //             display = lore::midware::host::X11::Display_G2H(display);
    //         }
    //     }
    // };

    // template <>
    // struct ProcReturnFilter<::Display *> {
    //     template <class Desc, ProcKind Kind, ProcDirection Direction, class... Args>
    //     static void filter(::Display *&display, ProcArgContext<Args...> ctx) {
    //         if constexpr (Direction == GuestToHost) {
    //             display = lore::midware::host::X11::Display_H2G(display, GTL_XOpenDisplay);
    //         }
    //     }
    // };

    // template <>
    // struct ProcArgFilter<::XVisualInfo *> {
    //     template <class Desc, size_t Index, ProcKind Kind, ProcDirection Direction, class... Args>
    //     static void filter(::XVisualInfo *&visualInfo, ProcArgContext<Args...> ctx) {
    //         if constexpr (Direction == GuestToHost) {
    //             Display *display = ctx.template type<::Display *>();
    //             visualInfo = lore::midware::host::X11::VisualInfo_G2H(display, visualInfo);
    //         }
    //     }
    // };

    // template <>
    // struct ProcReturnFilter<::XVisualInfo *> {
    //     template <class Desc, ProcKind Kind, ProcDirection Direction, class... Args>
    //     static void filter(::XVisualInfo *&visualInfo, ProcArgContext<Args...> ctx) {
    //         if constexpr (Direction == GuestToHost) {
    //             Display *display = ctx.template type<::Display *>();
    //             visualInfo = lore::midware::host::X11::VisualInfo_H2G(display, visualInfo);
    //         }
    //     }
    // };

}

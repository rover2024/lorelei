#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <cassert>

#include <lorelei/Tools/ThunkInterface/Proc.h>

namespace lore::thunk {

    template <>
    struct ProcArgFilter<::Display *> {
        template <class Desc, size_t Index, ProcKind Kind, ProcDirection Direction, class... Args>
        static void filter(::Display *&display, ProcArgContext<Args...> ctx) {
            if constexpr (Direction == GuestToHost) {
                XSync(display, false);
            }
        }
    };

    template <>
    struct ProcReturnFilter<::XVisualInfo *> {
        template <class Desc, ProcKind Kind, ProcDirection Direction, class... Args>
        static void filter(::XVisualInfo *&visualInfo, ProcArgContext<Args...> ctx) {
            if constexpr (Direction == GuestToHost) {
                Display *display = ctx.template type<::Display *>();
                int nitemsReturn = 0;
                visualInfo =
                    XGetVisualInfo(display, VisualScreenMask | VisualIDMask, visualInfo, &nitemsReturn);
                assert(nitemsReturn == 1);
                assert(visualInfo);
            }
        }
    };

}

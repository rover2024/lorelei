#ifndef LORE_X11_H
#define LORE_X11_H

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <lorelei/Modules/Midware/Host/X11/Global.h>

namespace lore::midware::host {

    struct X11 {
        static X11_HMW_EXPORT Display *Display_G2H(Display *display);
        static X11_HMW_EXPORT Display *Display_H2G(Display *display,
                                                   Display *(*guest_open)(const char *) );

        static X11_HMW_EXPORT XVisualInfo *VisualInfo_G2H(Display *display,
                                                          XVisualInfo *visualInfo);
        static X11_HMW_EXPORT XVisualInfo *VisualInfo_H2G(Display *display,
                                                          XVisualInfo *visualInfo);
    };

}

#endif // LORE_X11_H

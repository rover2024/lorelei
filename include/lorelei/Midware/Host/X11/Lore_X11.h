#ifndef LORE_X11_H
#define LORE_X11_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Global.h"

namespace lore::midware::host {

    struct X11 {
        static X11_HMW_EXPORT Display *Display_G2H(Display *display);
        static X11_HMW_EXPORT Display *Display_H2G(Display *display);

        static X11_HMW_EXPORT XVisualInfo *VisualInfo_G2H(XVisualInfo *visualInfo,
                                                          Display *display);
        static X11_HMW_EXPORT XVisualInfo *VisualInfo_H2G(XVisualInfo *visualInfo,
                                                          Display *display);
    };

}

#endif // LORE_X11_H

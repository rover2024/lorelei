#include "Lore_X11.h"

namespace lore::midware::host {

    Display *X11::Display_G2H(Display *display) {
        return display;
    }

    Display *X11::Display_H2G(Display *display) {
        return display;
    }

    XVisualInfo *X11::VisualInfo_G2H(XVisualInfo *visualInfo, Display *display) {
        return visualInfo;
    }

    XVisualInfo *X11::VisualInfo_H2G(XVisualInfo *visualInfo, Display *display) {
        return visualInfo;
    }

}
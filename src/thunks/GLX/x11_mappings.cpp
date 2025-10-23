#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>

#ifdef min
#  undef min
#endif

#ifdef max
#  undef max
#endif

extern "C" {
Display *X11Mappings_Display_G2H(Display *display);
Display *X11Mappings_Display_H2G(Display *display);

XVisualInfo *X11Mappings_VisualInfo_G2H(XVisualInfo *display);
XVisualInfo *X11Mappings_VisualInfo_H2G(XVisualInfo *display);
}

#include <mutex>
#include <unordered_map>

#define WARN(msg) ((void) 0)

namespace {

    std::mutex display_map_mu;
    std::unordered_map<Display *, Display *> h2g_display, g2h_display;
    __thread Display *last_guest_display;
    __thread Display *last_host_display;

    // https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L154
    Display *guestToHostDisplay(Display *guest_display) {
        if (guest_display == nullptr) {
            WARN("guestToHostDisplay(nil) called?");
            return nullptr;
        }
        std::lock_guard<std::mutex> guard(display_map_mu);
        if (auto it = g2h_display.find(guest_display); it != g2h_display.end()) {
            return it->second;
        }
        Display *host_display = XOpenDisplay(guest_display->display_name);
        if (host_display == nullptr) {
            WARN("Failed to open host display for guest display");
            return nullptr;
        }
        g2h_display[guest_display] = host_display;
        h2g_display[host_display] = guest_display;
        last_host_display = host_display;
        return host_display;
    }

    // https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L180
    Display *hostToGuestDisplay(Display *host_display) {
        if (host_display == nullptr) {
            WARN("hostToGuestDisplay(nil) called?");
            return nullptr;
        }
        std::lock_guard<std::mutex> guard(display_map_mu);
        if (auto it = h2g_display.find(host_display); it != h2g_display.end()) {
            return it->second;
        }
        WARN("Failed to open guest display for host display");
        return nullptr;
    }

}

Display *X11Mappings_Display_G2H(Display *guest_display) {
    last_guest_display = guest_display;
    last_host_display = guestToHostDisplay(guest_display);
    return last_host_display;
}

Display *X11Mappings_Display_H2G(Display *host_display) {
    last_host_display = host_display;
    last_guest_display = hostToGuestDisplay(host_display);
    return last_guest_display;
}

// https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L216
XVisualInfo *X11Mappings_VisualInfo_G2H(XVisualInfo *guest_info) {
    if (guest_info == nullptr) {
        return nullptr;
    }
    XVisualInfo v;
    v.screen = guest_info->screen;
    v.visualid = guest_info->visualid;
    int c;
    XVisualInfo *host_info =
        XGetVisualInfo(last_host_display, VisualScreenMask | VisualIDMask, &v, &c);
    if (c >= 1 && host_info != nullptr) {
        return host_info;
    }
    WARN("host XGetVisualInfo null");
    return nullptr;
}

// https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L196
XVisualInfo *X11Mappings_VisualInfo_H2G(XVisualInfo *host_info) {
    if (host_info == nullptr) {
        return nullptr;
    }
    static __thread XVisualInfo static_guest_info;
    auto &guest_info = static_guest_info;
    guest_info.screen = host_info->screen;
    guest_info.visualid = host_info->visualid;
    XFree(host_info);
    return &guest_info;
}
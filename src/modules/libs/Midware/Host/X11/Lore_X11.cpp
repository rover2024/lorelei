#include "Lore_X11.h"

#include <mutex>
#include <unordered_map>

#include <lorelei/Base/Support/Logging.h>
#define Success 0

extern "C" {
#include <X11/Xlibint.h>
}

#ifdef min
#  undef min
#endif

#ifdef max
#  undef max
#endif

namespace {

    std::mutex display_map_mu;
    std::unordered_map<Display *, Display *> h2g_display, g2h_display;

    // https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L154
    Display *guestToHostDisplay(Display *guest_display) {
        std::lock_guard<std::mutex> guard(display_map_mu);
        if (auto it = g2h_display.find(guest_display); it != g2h_display.end()) {
            return it->second;
        }
        Display *host_display = XOpenDisplay(guest_display->display_name);
        if (host_display == nullptr) {
            return nullptr;
        }
        g2h_display[guest_display] = host_display;
        h2g_display[host_display] = guest_display;
        return host_display;
    }

    // https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L180
    Display *hostToGuestDisplay(Display *host_display) {
        std::lock_guard<std::mutex> guard(display_map_mu);
        if (auto it = h2g_display.find(host_display); it != h2g_display.end()) {
            return it->second;
        }
        return nullptr;
    }

    // https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L216
    XVisualInfo *guestToHostVisualInfo(Display *host_display, XVisualInfo *guest_info) {
        XVisualInfo v;
        v.screen = guest_info->screen;
        v.visualid = guest_info->visualid;
        int c;
        auto host_info = XGetVisualInfo(host_display, VisualScreenMask | VisualIDMask, &v, &c);
        if (c >= 1 && host_info != nullptr) {
            return host_info;
        }
        loreWarningF("[HMW] X11: failed to find host visual info for guest visual info:"
                     "screen=%d, visualid=0x%08x",
                     guest_info->screen, guest_info->visualid);
        return nullptr;
    }

    // https://github.com/OFFTKP/felix86/blob/2c36eabb087b7963985e31137de6d0bbe29d0739/src/felix86/hle/thunks.cpp#L196
    XVisualInfo *hostToGuestVisualInfo(XVisualInfo *host_info) {
        if (host_info == nullptr) {
            return nullptr;
        }
        static thread_local XVisualInfo static_guest_info;
        auto &guest_info = static_guest_info;
        guest_info.screen = host_info->screen;
        guest_info.visualid = host_info->visualid;
        XFree(host_info);
        return &guest_info;
    }

}

namespace lore::midware::host {

    Display *X11::Display_G2H(Display *display) {
        if (!display) {
            return nullptr;
        }

        auto host_display = guestToHostDisplay(display);
        if (!host_display) {
            loreWarningF("[HMW] X11: failed to open host display for guest display: %s",
                         display->display_name);
            return nullptr;
        }
        return host_display;
    }

    Display *X11::Display_H2G(Display *display, Display *(*guest_open)(const char *) ) {
        if (!display) {
            return nullptr;
        }
        if (auto guest_display = hostToGuestDisplay(display); guest_display) {
            return guest_display;
        }
        if (guest_open) {
            auto guest_display = guest_open(display->display_name);
            if (guest_display) {
                std::ignore = guestToHostDisplay(guest_display);
                return guest_display;
            }
        }
        loreWarningF("[HMW] X11: failed to find guest display for host display: %s",
                     display->display_name);
        return nullptr;
    }

    XVisualInfo *X11::VisualInfo_G2H(Display *display, XVisualInfo *visualInfo) {
        if (!visualInfo) {
            return nullptr;
        }
        return guestToHostVisualInfo(display, visualInfo);
    }

    XVisualInfo *X11::VisualInfo_H2G(Display *display, XVisualInfo *visualInfo) {
        if (!visualInfo) {
            return nullptr;
        }
        return hostToGuestVisualInfo(visualInfo);
    }

}

#include "Lore_xcb.h"

#include <mutex>
#include <unordered_map>
#include <cstdio>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <xcb/xcb.h>

#include <stdcorelib/support/logging.h>

namespace {

    std::mutex display_map_mu;
    std::unordered_map<xcb_connection_t *, xcb_connection_t *> h2g_conn, g2h_conn;

    bool getDisplayName(xcb_connection_t *conn, char *buffer) {
        int fd = xcb_get_file_descriptor(conn);
        struct sockaddr_un addr;
        socklen_t len = sizeof(addr);

        int getsockname_ret = getpeername(fd, (struct sockaddr *) &addr, &len);
        if (getsockname_ret == -1) {
            return false;
        }
        if (addr.sun_family != AF_UNIX) {
            return false;
        }

        // Typical X11 Unix domain socket path: /tmp/.X11-unix/X<disp>
        const char prefix[] = "\0/tmp/.X11-unix/X";
        if (memcmp(addr.sun_path, prefix, sizeof(prefix) - 1) == 0) {
            buffer[0] = ':';
            strcpy(buffer + 1, addr.sun_path + (sizeof(prefix) - 1));
            return true;
        }
        return false;
    }

    xcb_connection_t *guestToHostConn(xcb_connection_t *guest_conn) {
        if (guest_conn == nullptr) {
            return nullptr;
        }
        std::lock_guard<std::mutex> guard(display_map_mu);
        if (auto it = g2h_conn.find(guest_conn); it != g2h_conn.end()) {
            return it->second;
        }

        xcb_connection_t *host_conn;
        if (char display_name[100]; getDisplayName(guest_conn, display_name)) {
            host_conn = xcb_connect(display_name, nullptr);
        } else {
            stdcWarningF("[HMW] xcb: failed to find host connection for guest connection: %p",
                         guest_conn);
            host_conn = xcb_connect(nullptr, nullptr);
        }

        if (host_conn == nullptr) {
            return nullptr;
        }
        g2h_conn[guest_conn] = host_conn;
        h2g_conn[host_conn] = guest_conn;
        return host_conn;
    }

    xcb_connection_t *hostToGuestConn(xcb_connection_t *host_conn) {
        if (host_conn == nullptr) {
            return nullptr;
        }
        std::lock_guard<std::mutex> guard(display_map_mu);
        if (auto it = h2g_conn.find(host_conn); it != h2g_conn.end()) {
            return it->second;
        }
        return nullptr;
    }

}

namespace lore::midware::host {

    xcb_connection_t *xcb::connection_G2H(xcb_connection_t *conn) {
        return guestToHostConn(conn);
    }

    xcb_connection_t *xcb::connection_H2G(xcb_connection_t *conn) {
        return hostToGuestConn(conn);
    }

}
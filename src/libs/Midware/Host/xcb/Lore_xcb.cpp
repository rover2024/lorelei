#include "Lore_xcb.h"

namespace lore::midware::host {

    xcb_connection_t *xcb::connection_G2H(xcb_connection_t *conn) {
        return conn;
    }

    xcb_connection_t *xcb::connection_H2G(xcb_connection_t *conn) {
        return conn;
    }

}
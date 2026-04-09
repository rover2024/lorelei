#ifndef LORE_XCB_H
#define LORE_XCB_H

typedef struct xcb_connection_t xcb_connection_t;

#include <lorelei/Modules/Midware/Host/xcb/Global.h>

namespace lore::midware::host {

    struct xcb {
        static XCB_HMW_EXPORT xcb_connection_t *connection_G2H(xcb_connection_t *conn);
        static XCB_HMW_EXPORT xcb_connection_t *connection_H2G(xcb_connection_t *conn);
    };

}

#endif // LORE_XCB_H

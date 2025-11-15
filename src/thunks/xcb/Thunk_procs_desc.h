#pragma once

#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/bigreq.h>
#include <xcb/xc_misc.h>
#include <xcb/xcbext.h>
#include <xcb/xproto.h>
#include <xcb/render.h>
#include <xcb/shm.h>
#include <xcb/randr.h>
#include <xcb/dri2.h>
#include <xcb/dri3.h>
#include <xcb/xfixes.h>
#include <xcb/shape.h>
#include <xcb/sync.h>
#include <xcb/present.h>
#include <xcb/glx.h>
// #include <xcb/xcb_bitops.h>
#include <xcb/xcb_image.h>
// #include <xcb/xcb_pixel.h>
#include <xcb/xcb_renderutil.h>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_User> {
        _DESC char moduleName[] = "libxcb";
    };

}
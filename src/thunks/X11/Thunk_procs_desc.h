#pragma once

extern "C" {
#define XUTIL_DEFINE_FUNCTIONS
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xcms.h>
#include <X11/XlibConf.h>
#include <X11/Xlocale.h>
#include <X11/Xregion.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/ImUtil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>
}

#ifdef max
#  undef max
#endif

#ifdef min
#  undef min
#endif


#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_User> {
        _DESC char moduleName[] = "libX11";
    };

}
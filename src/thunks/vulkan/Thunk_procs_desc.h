#pragma once

#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR

#include <vulkan/vulkan.h>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_User> {
        _DESC char moduleName[] = "libvulkan";
    };

}
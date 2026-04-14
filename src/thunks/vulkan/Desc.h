#pragma once

#define VK_USE_PLATFORM_XLIB_KHR
#define VK_USE_PLATFORM_XCB_KHR
#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

#ifdef Success
#  undef Success
#endif

#include <lorelei/Tools/ThunkInterface/Proc.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>

namespace lore::thunk {}

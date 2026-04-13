#include "Desc.h"
#include <lorelei/Tools/ThunkInterface/Host/ManifestDef.cpp.inc>

#define LORE_THUNK_CALLBACK_REPLACE

#include <cstring>

#include <lorelei/Modules/Midware/Host/X11/Lore_X11.h>
#include <lorelei/Modules/Midware/Host/xcb/Lore_xcb.h>

namespace lore::thunk {

    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName);

    template <>
    struct ProcFn<::vkCreateDebugReportCallbackEXT, GuestToHost, Caller> {
        static VkResult invoke(VkInstance instance,
                               const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator,
                               VkDebugReportCallbackEXT *pCallback) {
            return VK_SUCCESS;
        }
    };

    template <>
    struct ProcFn<::vkDestroyDebugReportCallbackEXT, GuestToHost, Caller> {
        static void invoke(VkInstance instance, VkDebugReportCallbackEXT callback,
                           const VkAllocationCallbacks *pAllocator) {
        }
    };

    template <>
    struct ProcFn<::vkCreateInstance, GuestToHost, Caller> {
        static VkResult invoke(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
            // Remove debug callbacks from VkInstanceCreateInfo.
            VkBaseInStructure *base = (VkBaseInStructure *) pCreateInfo;
            while (base->pNext) {
                VkBaseInStructure *next = (VkBaseInStructure *) base->pNext;
                if (next->sType == VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT) {
                    base->pNext = next->pNext;
                    if (!base->pNext) {
                        break;
                    }
                }
                base = (VkBaseInStructure *) base->pNext;
            }
            return ProcFn<vkCreateInstance, GuestToHost, Exec>::invoke(pCreateInfo, pAllocator,
                                                                        pInstance);
        }
    };

    template <>
    struct ProcFn<::vkGetInstanceProcAddr, GuestToHost, Caller> {
        static PFN_vkVoidFunction invoke(VkInstance instance, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (!ret) {
                ret = ProcFn<vkGetInstanceProcAddr, GuestToHost, Exec>::invoke(instance, pName);
            }
            return ret;
        }
    };

    template <>
    struct ProcFn<::vkGetDeviceProcAddr, GuestToHost, Caller> {
        static PFN_vkVoidFunction invoke(VkDevice device, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (!ret) {
                ret = ProcFn<vkGetDeviceProcAddr, GuestToHost, Exec>::invoke(device, pName);
            }
            return ret;
        }
    };

    template <>
    struct ProcCb<PFN_vkCreateXcbSurfaceKHR, GuestToHost, Caller> {
        static VkResult invoke(void *callback, VkInstance instance,
                               const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
            // Replace guest xcb connection pointers with host ones.
            int replacedConnCnt = 0;
            struct ReplacedConnection {
                xcb_connection_t **ptr;
                xcb_connection_t *oldConn;
            };
            struct ReplacedConnection replacedConnList[50];
            auto info = (VkXcbSurfaceCreateInfoKHR *) pCreateInfo;
            while (info) {
                if (info->connection) {
                    struct ReplacedConnection r;
                    r.ptr = (xcb_connection_t **) &info->connection;
                    r.oldConn = *r.ptr;
                    *r.ptr = lore::midware::host::xcb::connection_G2H(r.oldConn);
                    replacedConnList[replacedConnCnt++] = r;
                }
                info = (VkXcbSurfaceCreateInfoKHR *) info->pNext;
            }

            using CallbackType =
                VkResult (*)(VkInstance, const VkXcbSurfaceCreateInfoKHR *,
                             const VkAllocationCallbacks *, VkSurfaceKHR *);
            VkResult ret = reinterpret_cast<CallbackType>(callback)(instance, pCreateInfo,
                                                                    pAllocator, pSurface);

            // Restore guest pointers.
            for (int i = 0; i < replacedConnCnt; i++) {
                *replacedConnList[i].ptr = replacedConnList[i].oldConn;
            }
            return ret;
        }
    };

    template <>
    struct ProcCb<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR, GuestToHost, Caller> {
        static VkBool32 invoke(void *callback, VkPhysicalDevice physicalDevice,
                               uint32_t queueFamilyIndex, xcb_connection_t *connection,
                               xcb_visualid_t visualId) {
            using CallbackType =
                VkBool32 (*)(VkPhysicalDevice, uint32_t, xcb_connection_t *, xcb_visualid_t);
            VkBool32 ret =
                reinterpret_cast<CallbackType>(callback)(
                    physicalDevice, queueFamilyIndex,
                    lore::midware::host::xcb::connection_G2H(connection), visualId);
            return ret;
        }
    };

    template <>
    struct ProcCb<PFN_vkCreateXlibSurfaceKHR, GuestToHost, Caller> {
        static VkResult invoke(void *callback, VkInstance instance,
                               const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
            // Replace guest X11 display pointers with host ones.
            int replacedDpyCnt = 0;
            struct ReplacedDisplay {
                Display **ptr;
                Display *oldDpy;
            };
            struct ReplacedDisplay replacedDpyList[50];
            auto info = (VkXlibSurfaceCreateInfoKHR *) pCreateInfo;
            while (info) {
                if (info->dpy) {
                    struct ReplacedDisplay r;
                    r.ptr = (Display **) &info->dpy;
                    r.oldDpy = *r.ptr;
                    *r.ptr = lore::midware::host::X11::Display_G2H(r.oldDpy);
                    replacedDpyList[replacedDpyCnt++] = r;
                }
                info = (VkXlibSurfaceCreateInfoKHR *) info->pNext;
            }

            using CallbackType =
                VkResult (*)(VkInstance, const VkXlibSurfaceCreateInfoKHR *,
                             const VkAllocationCallbacks *, VkSurfaceKHR *);
            VkResult ret = reinterpret_cast<CallbackType>(callback)(instance, pCreateInfo,
                                                                    pAllocator, pSurface);

            // Restore guest pointers.
            for (int i = 0; i < replacedDpyCnt; i++) {
                *replacedDpyList[i].ptr = replacedDpyList[i].oldDpy;
            }
            return ret;
        }
    };

    template <>
    struct ProcCb<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR, GuestToHost, Caller> {
        static VkBool32 invoke(void *callback, VkPhysicalDevice physicalDevice,
                               uint32_t queueFamilyIndex, Display *dpy, VisualID visualId) {
            using CallbackType = VkBool32 (*)(VkPhysicalDevice, uint32_t, Display *, VisualID);
            VkBool32 ret =
                reinterpret_cast<CallbackType>(callback)(
                    physicalDevice, queueFamilyIndex,
                    lore::midware::host::X11::Display_G2H(dpy), visualId);
            return ret;
        }
    };

    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName) {
        if (std::strcmp(pName, "vkCreateDebugReportCallbackEXT") == 0) {
            return (PFN_vkVoidFunction)
                ProcFn<vkCreateDebugReportCallbackEXT, GuestToHost, Caller>::invoke;
        } else if (std::strcmp(pName, "vkDestroyDebugReportCallbackEXT") == 0) {
            return (PFN_vkVoidFunction)
                ProcFn<vkDestroyDebugReportCallbackEXT, GuestToHost, Caller>::invoke;
        } else if (std::strcmp(pName, "vkCreateInstance") == 0) {
            return (PFN_vkVoidFunction) ProcFn<vkCreateInstance, GuestToHost, Caller>::invoke;
        } else if (std::strcmp(pName, "vkGetInstanceProcAddr") == 0) {
            return (PFN_vkVoidFunction) ProcFn<vkGetInstanceProcAddr, GuestToHost, Caller>::invoke;
        } else if (std::strcmp(pName, "vkGetDeviceProcAddr") == 0) {
            return (PFN_vkVoidFunction) ProcFn<vkGetDeviceProcAddr, GuestToHost, Caller>::invoke;
        }
        return nullptr;
    }

}

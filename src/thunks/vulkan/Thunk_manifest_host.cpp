#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_host.inc.h>

// #define LORETHUNK_CALLBACK_REPLACE

#include <cstring>

#include <lorelei/Midware/Host/X11/Lore_X11.h>
#include <lorelei/Midware/Host/xcb/Lore_xcb.h>

namespace lorethunk {

    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName);

    template <>
    struct MetaProc<vkCreateDebugReportCallbackEXT, _HFN, _HTP_IMPL> {
        static VkResult invoke(VkInstance instance,
                               const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator,
                               VkDebugReportCallbackEXT *pCallback) {
            return VK_SUCCESS;
        }
    };

    template <>
    struct MetaProc<vkDestroyDebugReportCallbackEXT, _HFN, _HTP_IMPL> {
        static void invoke(VkInstance instance, VkDebugReportCallbackEXT callback,
                           const VkAllocationCallbacks *pAllocator) {
        }
    };

    template <>
    struct MetaProc<vkCreateInstance, _HFN, _HTP_IMPL> {
        static VkResult invoke(const VkInstanceCreateInfo *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
            // Remove debug callbacks from VkInstanceCreateInfo
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
            return MetaProcExec<vkCreateInstance, _HFN>::invoke(pCreateInfo, pAllocator, pInstance);
        }
    };

    template <>
    struct MetaProc<vkGetInstanceProcAddr, _HFN, _HTP_IMPL> {
        static PFN_vkVoidFunction invoke(VkInstance instance, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (!ret) {
                ret = MetaProcExec<vkGetInstanceProcAddr, _HFN>::invoke(instance, pName);
            }
            return ret;
        }
    };

    template <>
    struct MetaProc<vkGetDeviceProcAddr, _HFN, _HTP_IMPL> {
        static PFN_vkVoidFunction invoke(VkDevice device, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (!ret) {
                ret = MetaProcExec<vkGetDeviceProcAddr, _HFN>::invoke(device, pName);
            }
            return ret;
        }
    };

    template <>
    struct MetaProcCB<PFN_vkCreateXcbSurfaceKHR, _HCB, _HTP_IMPL> {
        static VkResult invoke(void *callback, VkInstance instance,
                               const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {

            // replace
            int replaced_conn_cnt = 0;
            struct ReplacedConnection {
                xcb_connection_t **ptr;
                xcb_connection_t *old_conn;
            };
            struct ReplacedConnection replaced_conn_list[50];
            VkXcbSurfaceCreateInfoKHR *info = (VkXcbSurfaceCreateInfoKHR *) pCreateInfo;
            while (info) {
                if (info->connection) {
                    struct ReplacedConnection r;
                    r.ptr = (xcb_connection_t **) &info->connection;
                    r.old_conn = *r.ptr;
                    *r.ptr = lore::midware::host::xcb::connection_G2H(r.old_conn);
                    replaced_conn_list[replaced_conn_cnt++] = r;
                }
                info = (VkXcbSurfaceCreateInfoKHR *) info->pNext;
            }

            VkResult ret;
            ret =
                ((__typeof__(VkResult(VkInstance, const VkXcbSurfaceCreateInfoKHR *,
                                      const VkAllocationCallbacks *, VkSurfaceKHR *)) *) callback)(
                    instance, pCreateInfo, pAllocator, pSurface);

            // restore
            for (int i = 0; i < replaced_conn_cnt; i++) {
                *replaced_conn_list[i].ptr = replaced_conn_list[i].old_conn;
            }
            return ret;
        }
    };

    template <>
    struct MetaProcCB<PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR, _HCB, _HTP_IMPL> {
        static VkBool32 invoke(void *callback, VkPhysicalDevice physicalDevice,
                               uint32_t queueFamilyIndex, xcb_connection_t *connection,
                               xcb_visualid_t visual_id) {
            VkBool32 ret;
            ret = ((__typeof__(VkBool32(VkPhysicalDevice, uint32_t, xcb_connection_t *,
                                        xcb_visualid_t)) *) callback)(
                physicalDevice, queueFamilyIndex,
                lore::midware::host::xcb::connection_G2H(connection), visual_id);
            return ret;
        }
    };

    template <>
    struct MetaProcCB<PFN_vkCreateXlibSurfaceKHR, _HCB, _HTP_IMPL> {
        static VkResult invoke(void *callback, VkInstance instance,
                               const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
            // replace
            int replaced_dpy_cnt = 0;
            struct ReplacedDisplay {
                Display **ptr;
                Display *old_dpy;
            };
            struct ReplacedDisplay replaced_dpy_list[50];
            VkXlibSurfaceCreateInfoKHR *info = (VkXlibSurfaceCreateInfoKHR *) pCreateInfo;
            while (info) {
                if (info->dpy) {
                    struct ReplacedDisplay r;
                    r.ptr = (Display **) &info->dpy;
                    r.old_dpy = *r.ptr;
                    *r.ptr = lore::midware::host::X11::Display_G2H(r.old_dpy);
                    replaced_dpy_list[replaced_dpy_cnt++] = r;
                }
                info = (VkXlibSurfaceCreateInfoKHR *) info->pNext;
            }

            VkResult ret;
            ret =
                ((__typeof__(VkResult(VkInstance, const VkXlibSurfaceCreateInfoKHR *,
                                      const VkAllocationCallbacks *, VkSurfaceKHR *)) *) callback)(
                    instance, pCreateInfo, pAllocator, pSurface);

            // restore
            for (int i = 0; i < replaced_dpy_cnt; i++) {
                *replaced_dpy_list[i].ptr = replaced_dpy_list[i].old_dpy;
            }
            return ret;
        }
    };

    template <>
    struct MetaProcCB<PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR, _HCB, _HTP_IMPL> {
        static VkBool32 invoke(void *callback, VkPhysicalDevice physicalDevice,
                               uint32_t queueFamilyIndex, Display *dpy, VisualID visualID) {
            VkBool32 ret;
            ret = ((__typeof__(VkBool32(VkPhysicalDevice, uint32_t, Display *, VisualID)) *)
                       callback)(physicalDevice, queueFamilyIndex,
                                 lore::midware::host::X11::Display_G2H(dpy), visualID);
            return ret;
        }
    };

    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName) {
        if (std::strcmp(pName, "vkCreateDebugReportCallbackEXT") == 0) {
            return (PFN_vkVoidFunction)
                MetaProc<vkCreateDebugReportCallbackEXT, _HFN, _HTP_IMPL>::invoke;
        } else if (std::strcmp(pName, "vkDestroyDebugReportCallbackEXT") == 0) {
            return (PFN_vkVoidFunction)
                MetaProc<vkDestroyDebugReportCallbackEXT, _HFN, _HTP_IMPL>::invoke;
        } else if (std::strcmp(pName, "vkCreateInstance") == 0) {
            return (PFN_vkVoidFunction) MetaProc<vkCreateInstance, _HFN, _HTP_IMPL>::invoke;
        } else if (std::strcmp(pName, "vkGetInstanceProcAddr") == 0) {
            return (PFN_vkVoidFunction) MetaProc<vkGetInstanceProcAddr, _HFN, _HTP_IMPL>::invoke;
        } else if (std::strcmp(pName, "vkGetDeviceProcAddr") == 0) {
            return (PFN_vkVoidFunction) MetaProc<vkGetDeviceProcAddr, _HFN, _HTP_IMPL>::invoke;
        }
        return nullptr;
    }

}

#include <dlfcn.h>

#include "VulkanApis.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lorelei/loreshared.h>
#include <lorelei/loreuser.h>

#include <lorelib_common/manifest-predef.h>
#include <lorelib_common/callback.h>

static int first = 0;



//
// Option macros
//
#define LORELIB_GCB_AUTO_DEPTH 1
#define LORELIB_HCB_AUTO_DEPTH 1
// #define LORELIB_CALLBACK_REPLACE



//
// Annotations
//
#ifdef LORELIB_VISUAL

#endif



//
// Custom(Guest)
//
#if defined(LORELIB_GTL_BUILD) || defined(LORELIB_VISUAL)

static const VkAllocationCallbacks *__TYPECONV_G_G2H_VkAllocationCallbacks(const VkAllocationCallbacks *pAllocator) {
    return NULL;
}

static PFN_vkVoidFunction Vulkan_GetGTLHijackedProcAddressHelper(const char *pName);

#  define _F(NAME)                                                                                 \
      LORELIB_HCB_THUNK_ALLOCATOR(__GTP_HCB_PFN_##NAME##_xx_ThunkAlloc, __GTP_HCB_PFN_##NAME)
LORELIB_VULKAN_API_FOREACH(_F)
#  undef _F

struct VulkanApiTableEntry {
    const char *name;
    void *pfn;
};
static struct VulkanApiTableEntry vulkanApiTable[] = {
#  define _F(NAME) {#NAME, __GTP_HCB_PFN_##NAME##_xx_ThunkAlloc},
    LORELIB_VULKAN_API_FOREACH(_F)
#  undef _F
        { NULL, NULL},
};

struct VulkanApiTableEntry *vulkanApiTablePtr = vulkanApiTable;

void *vulkanApiTableGet(const char *pName);

#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wundefined-internal"

static PFN_vkVoidFunction ___GTP_vkGetDeviceProcAddr(VkDevice device, const char *pName);
static PFN_vkVoidFunction ___GTP_vkGetInstanceProcAddr(VkInstance instance, const char *pName);

#  pragma clang diagnostic pop

#  define LOG_ERROR(fmt, ...)                                                                      \
      ({                                                                                           \
        FILE *fp = fopen("/tmp/1.txt", (first ? "a" : (first = 1, "w")));                          \
        fprintf(fp, fmt, ##__VA_ARGS__);                                                           \
        fclose(fp);                                                                                \
      })

static PFN_vkVoidFunction __GTP_vkGetDeviceProcAddr(VkDevice device, const char *pName) {
    // void *result = vulkan_GetProcAddressHelper(pName);
    // FILE *fp = fopen("/tmp/1.txt", (first ? "a" : (first = 1, "w")));
    // fprintf(fp, "vkGetDeviceProcAddr: %s, %p\n", pName, result);
    // fclose(fp);
    // return result;

    PFN_vkVoidFunction ret = Vulkan_GetGTLHijackedProcAddressHelper(pName);
    if (ret) {
        return ret;
    }
    ret = ___GTP_vkGetDeviceProcAddr(device, pName);
    if (!ret) {
        return NULL;
    }
    void *(*pFnAlloc)(void *) = vulkanApiTableGet(pName);
    if (!pFnAlloc) {
        LOG_ERROR("Failed to find function %s in vulkanApiTable\n", pName);
        abort();
        return NULL;
    }
    ret = pFnAlloc((void *) ret);
    LOG_ERROR("[GUEST] Get device proc addr: %s, %p\n", pName, ret);
    return ret;
}
static PFN_vkVoidFunction __GTP_vkGetInstanceProcAddr(VkInstance instance, const char *pName) {
    // void *result = vulkan_GetProcAddressHelper(pName);
    // FILE *fp = fopen("/tmp/1.txt", (first ? "a" : (first = 1, "w")));
    // fprintf(fp, "vkGetInstanceProcAddr: %s, %p\n", pName, result);
    // fclose(fp);
    // return result;

    PFN_vkVoidFunction ret = Vulkan_GetGTLHijackedProcAddressHelper(pName);
    if (ret) {
        return ret;
    }
    ret = ___GTP_vkGetInstanceProcAddr(instance, pName);
    if (!ret) {
        return NULL;
    }
    void *(*pFnAlloc)(void *) = vulkanApiTableGet(pName);
    if (!pFnAlloc) {
        LOG_ERROR("Failed to find function %s in vulkanApiTable\n", pName);
        abort();
        return NULL;
    }
    ret = pFnAlloc((void *) ret);
    LOG_ERROR("[GUEST] Get instance proc addr: %s, %p\n", pName, ret);
    return ret;
}

// Helpers
static PFN_vkVoidFunction Vulkan_GetGTLHijackedProcAddressHelper(const char *pName) {
    if (strcmp(pName, "vkGetInstanceProcAddr") == 0) {
        return (PFN_vkVoidFunction) __GTP_vkGetInstanceProcAddr;
    } else if (strcmp(pName, "vkGetDeviceProcAddr") == 0) {
        return (PFN_vkVoidFunction) __GTP_vkGetDeviceProcAddr;
    }
    return NULL;
}
#endif



//
// Custom(Host)
//
#if defined(LORELIB_HTL_BUILD) || defined(LORELIB_VISUAL)

#  define LOG_ERROR(fmt, ...)                                                                      \
      ({                                                                                           \
        FILE *fp = fopen("/tmp/1.txt", (first ? "a" : (first = 1, "w")));                          \
        fprintf(fp, fmt, ##__VA_ARGS__);                                                           \
        fclose(fp);                                                                                \
      })

static PFN_vkVoidFunction Vulkan_GetHTLHijackedProcAddressHelper(const char *pName);

static VkResult ___HTP_vkCreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback) {
    return VK_SUCCESS;
}
static void ___HTP_vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                                   VkDebugReportCallbackEXT callback,
                                                   const VkAllocationCallbacks *pAllocator) {
    // Do nothing
}
static VkResult ___HTP_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator,
                                        VkInstance *pInstance) {
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
    VkResult ret;
    ret = LORELIB_API(vkCreateInstance)(pCreateInfo, pAllocator, pInstance);
    return ret;
}
static PFN_vkVoidFunction ___HTP_vkGetInstanceProcAddr(VkInstance instance, const char *pName) {
    PFN_vkVoidFunction ret = Vulkan_GetHTLHijackedProcAddressHelper(pName);
    if (!ret) {
        ret = LORELIB_API(vkGetInstanceProcAddr)(instance, pName);
    }
    LOG_ERROR("[HOST] Get instance proc addr: %s, %p\n", pName, ret);
    return ret;
}
static PFN_vkVoidFunction ___HTP_vkGetDeviceProcAddr(VkDevice device, const char *pName) {
    PFN_vkVoidFunction ret = Vulkan_GetHTLHijackedProcAddressHelper(pName);
    if (!ret) {
        ret = LORELIB_API(vkGetDeviceProcAddr)(device, pName);
    }
    LOG_ERROR("[HOST] Get device proc addr: %s, %p\n", pName, ret);
    return ret;
}

static xcb_connection_t *xcb_conn;

static VkResult ___HTP_HCB_PFN_vkCreateXcbSurfaceKHR(void *callback, VkInstance instance,
                                                     const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator,
                                                     VkSurfaceKHR *pSurface) {
    if (!xcb_conn) {
        xcb_conn = xcb_connect(NULL, NULL);
    }

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
            *r.ptr = xcb_conn;
            replaced_conn_list[replaced_conn_cnt++] = r;
        }
        info = (VkXcbSurfaceCreateInfoKHR *) info->pNext;
    }

    VkResult ret;
    ret = ((__typeof__(VkResult(VkInstance, const VkXcbSurfaceCreateInfoKHR *,
                                const VkAllocationCallbacks *, VkSurfaceKHR *)) *) callback)(
        instance, pCreateInfo, pAllocator, pSurface);

    for (int i = 0; i < replaced_conn_cnt; i++) {
        *replaced_conn_list[i].ptr = replaced_conn_list[i].old_conn;
    }
    return ret;
}

static VkBool32 ___HTP_HCB_PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR(
    void *callback, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex,
    xcb_connection_t *connection, xcb_visualid_t visual_id) {
    if (!xcb_conn) {
        xcb_conn = xcb_connect(NULL, NULL);
    }

    VkBool32 ret;
    ret = ((__typeof__(VkBool32(VkPhysicalDevice, uint32_t, xcb_connection_t *, xcb_visualid_t)) *)
               callback)(physicalDevice, queueFamilyIndex, xcb_conn, visual_id);
    return ret;
}

extern Display *X11Mappings_Display_G2H(Display *display);
extern Display *X11Mappings_Display_H2G(Display *display);

static VkResult ___HTP_HCB_PFN_vkCreateXlibSurfaceKHR(void *callback, VkInstance instance,
                                                      const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator,
                                                      VkSurfaceKHR *pSurface) {
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
            *r.ptr = X11Mappings_Display_G2H(r.old_dpy);
            replaced_dpy_list[replaced_dpy_cnt++] = r;
        }
        info = (VkXlibSurfaceCreateInfoKHR *) info->pNext;
    }

    VkResult ret;
    ret = ((__typeof__(VkResult(VkInstance, const VkXlibSurfaceCreateInfoKHR *,
                                const VkAllocationCallbacks *, VkSurfaceKHR *)) *) callback)(
        instance, pCreateInfo, pAllocator, pSurface);

    for (int i = 0; i < replaced_dpy_cnt; i++) {
        *replaced_dpy_list[i].ptr = replaced_dpy_list[i].old_dpy;
    }
    return ret;
}

static VkBool32 ___HTP_HCB_PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR(
    void *callback, VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, Display *dpy,
    VisualID visualID) {
    VkBool32 ret;
    ret = ((__typeof__(VkBool32(VkPhysicalDevice, uint32_t, Display *, VisualID)) *) callback)(
        physicalDevice, queueFamilyIndex, X11Mappings_Display_G2H(dpy), visualID);
    return ret;
}

// Helpers
static PFN_vkVoidFunction Vulkan_GetHTLHijackedProcAddressHelper(const char *pName) {
    if (strcmp(pName, "vkCreateDebugReportCallbackEXT") == 0) {
        return (PFN_vkVoidFunction) ___HTP_vkCreateDebugReportCallbackEXT;
    } else if (strcmp(pName, "vkDestroyDebugReportCallbackEXT") == 0) {
        return (PFN_vkVoidFunction) ___HTP_vkDestroyDebugReportCallbackEXT;
    } else if (strcmp(pName, "vkCreateInstance") == 0) {
        return (PFN_vkVoidFunction) ___HTP_vkCreateInstance;
    } else if (strcmp(pName, "vkGetInstanceProcAddr") == 0) {
        return (PFN_vkVoidFunction) ___HTP_vkGetInstanceProcAddr;
    } else if (strcmp(pName, "vkGetDeviceProcAddr") == 0) {
        return (PFN_vkVoidFunction) ___HTP_vkGetDeviceProcAddr;
    }
    return NULL;
}


#endif

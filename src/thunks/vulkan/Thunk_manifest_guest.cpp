#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

#define LORETHUNK_CALLBACK_REPLACE

#include <map>
#include <string>
#include <cstring>

#ifdef LORETHUNK_BUILD
#  include "VulkanApis.h"
#else
#  define LORETHUNK_VULKAN_API_FOREACH(F)
#endif

namespace lorethunk {

    enum class VulkanApiConstant {
        MaxLayerCount = 8,
    };

    enum class VulkanApi {
#define _F(NAME) NAME,
        LORETHUNK_VULKAN_API_FOREACH(_F)
#undef _F
            NumVulkanApi,
    };

    template <VulkanApi Api>
    struct VulkanApiTrampolineContext {
        int num = (int) Api;
    };

    static struct CStaticProcInfo vulkanApiTable[] = {
#define _F(NAME)                                                                                   \
    {                                                                                              \
        #NAME,                                                                                     \
        (void *) allocCallbackTrampoline<MetaProcCB<PFN_##NAME, _HCB, _GTP>::invoke,               \
                                         VulkanApiTrampolineContext<VulkanApi::NAME>,              \
                                         (size_t) VulkanApiConstant::MaxLayerCount>,               \
    },
        LORETHUNK_VULKAN_API_FOREACH(_F)
#undef _F
            {nullptr, nullptr},
    };

    static void *vulkanApiTrampolineGet(const char *pName) {
        static bool initialized = false;
        static std::map<std::string, void *> table;
        if (!initialized) {
            for (auto p = vulkanApiTable; p->name; ++p) {
                table[p->name] = p->addr;
            }
            initialized = true;
        };
        auto it = table.find(pName);
        if (it == table.end()) {
            return nullptr;
        }
        return it->second;
    };

    template <>
    struct MetaProcArgFilter<const ::VkAllocationCallbacks *> {
        template <class MetaDesc, size_t Index, CProcKind ProcKind, class... Args>
        static void filter(const VkAllocationCallbacks *&pAllocator,
                           MetaProcArgContext<Args...> ctx) {
            pAllocator = nullptr;
        }
    };

    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName);

    template <>
    struct MetaProc<::vkGetDeviceProcAddr, _HFN, _GTP> {
        static PFN_vkVoidFunction invoke(VkDevice device, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (ret) {
                return ret;
            }
            ret = MetaProc<vkGetDeviceProcAddr, _HFN, _GTP_IMPL>::invoke(device, pName);
            if (!ret) {
                return nullptr;
            }
            auto pFnAlloc = (void *(*) (void *) ) vulkanApiTrampolineGet(pName);
            if (!pFnAlloc) {
                std::abort();
                return nullptr;
            }
            ret = (PFN_vkVoidFunction) pFnAlloc((void *) ret);
            return ret;
        }
    };

    template <>
    struct MetaProc<::vkGetInstanceProcAddr, _HFN, _GTP> {
        static PFN_vkVoidFunction invoke(VkInstance instance, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (ret) {
                return ret;
            }
            ret = MetaProc<vkGetInstanceProcAddr, _HFN, _GTP_IMPL>::invoke(instance, pName);
            if (!ret) {
                return nullptr;
            }
            auto pFnAlloc = (void *(*) (void *) ) vulkanApiTrampolineGet(pName);
            if (!pFnAlloc) {
                std::abort();
                return nullptr;
            }
            void *host_ret = (void *) ret;
            ret = (PFN_vkVoidFunction) pFnAlloc((void *) ret);
            return ret;
        }
    };

    // Helpers
    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName) {
        if (std::strcmp(pName, "vkGetInstanceProcAddr") == 0) {
            return (PFN_vkVoidFunction) MetaProc<vkGetInstanceProcAddr, _HFN, _GTP>::invoke;
        } else if (std::strcmp(pName, "vkGetDeviceProcAddr") == 0) {
            return (PFN_vkVoidFunction) MetaProc<vkGetDeviceProcAddr, _HFN, _GTP>::invoke;
        }
        return nullptr;
    }

}

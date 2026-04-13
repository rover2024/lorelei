#include "Desc.h"

#ifdef Success
#  undef Success
#endif

#include <lorelei/Tools/ThunkInterface/Guest/ManifestDef.cpp.inc>

#define LORE_THUNK_CALLBACK_REPLACE

#include <map>
#include <string>
#include <cstring>
#include <cstdlib>

#ifdef LORE_THUNK_BUILD
#  include "VulkanApis.h"
#else
#  define LORE_THUNK_VULKAN_API_FOREACH(F)
#endif

namespace lore::thunk {

    enum class VulkanApiConstant {
        MaxLayerCount = 8,
    };

    enum class VulkanApi {
#define _F(NAME) NAME,
        LORE_THUNK_VULKAN_API_FOREACH(_F)
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
        (void *) allocCallbackTrampoline<ProcCb<PFN_##NAME, GuestToHost, Entry>::invoke,           \
                                         VulkanApiTrampolineContext<VulkanApi::NAME>,              \
                                         (size_t) VulkanApiConstant::MaxLayerCount>,               \
    },
        LORE_THUNK_VULKAN_API_FOREACH(_F)
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
    struct ProcArgFilter<const ::VkAllocationCallbacks *> {
        template <class Desc, size_t Index, ProcKind Kind, ProcDirection Direction, class... Args>
        static void filter(const VkAllocationCallbacks *&pAllocator, ProcArgContext<Args...> ctx) {
            pAllocator = nullptr;
        }
    };

    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName);

    template <>
    struct ProcFn<::vkGetDeviceProcAddr, GuestToHost, Entry> {
        static PFN_vkVoidFunction invoke(VkDevice device, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (ret) {
                return ret;
            }
            ret = ProcFn<vkGetDeviceProcAddr, GuestToHost, Caller>::invoke(device, pName);
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
    struct ProcFn<::vkGetInstanceProcAddr, GuestToHost, Entry> {
        static PFN_vkVoidFunction invoke(VkInstance instance, const char *pName) {
            PFN_vkVoidFunction ret = vulkanApiGetReservedProc(pName);
            if (ret) {
                return ret;
            }
            ret = ProcFn<vkGetInstanceProcAddr, GuestToHost, Caller>::invoke(instance, pName);
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

    static PFN_vkVoidFunction vulkanApiGetReservedProc(const char *pName) {
        if (std::strcmp(pName, "vkGetInstanceProcAddr") == 0) {
            return (PFN_vkVoidFunction) ProcFn<vkGetInstanceProcAddr, GuestToHost, Entry>::invoke;
        } else if (std::strcmp(pName, "vkGetDeviceProcAddr") == 0) {
            return (PFN_vkVoidFunction) ProcFn<vkGetDeviceProcAddr, GuestToHost, Entry>::invoke;
        }
        return nullptr;
    }

}

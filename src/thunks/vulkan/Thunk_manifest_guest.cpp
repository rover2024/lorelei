#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

#include <dlfcn.h>

// #define LORETHUNK_CALLBACK_REPLACE
#include <map>
#include <string>

#ifdef LORETHUNK_BUILD
#  include "VulkanApis.h"
#else
#  define LORETHUNK_VULKAN_API_FOREACH(F)
#endif

namespace lorethunk {

    enum class VulkanApi {
#define _F(NAME) NAME,
        LORETHUNK_VULKAN_API_FOREACH(_F)
#undef _F
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
                                         VulkanApiTrampolineContext<VulkanApi::NAME>, 4>,          \
    },
        LORETHUNK_VULKAN_API_FOREACH(_F)
#undef _F
            {nullptr, nullptr},
    };

    static void *vulkanApiTableGet(const char *pName) {
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
        template <class MetaDesc, size_t Index, class... Args>
        static void filter(const VkAllocationCallbacks *&pAllocator,
                           MetaProcArgContext<Args...> ctx) {
            pAllocator = nullptr;
        }
    };

}
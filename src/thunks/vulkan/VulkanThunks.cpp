#include "VulkanApis.h"

#include <map>
#include <string>

#include <xcb/xcb.h>

extern "C" {

struct VulkanApiTableEntry {
    const char *name;
    void *pfn;
};

extern struct VulkanApiTableEntry *vulkanApiTablePtr;

void *vulkanApiTableGet(const char *pName) {
    static bool initialized = false;
    static std::map<std::string, void *> vulkanApiTableMap;
    if (!initialized) {
        for (auto p = vulkanApiTablePtr; p->name; ++p) {
            vulkanApiTableMap[p->name] = p->pfn;
        }
        initialized = true;
    };

    auto it = vulkanApiTableMap.find(pName);
    if (it == vulkanApiTableMap.end()) {
        return nullptr;
    }
    return it->second;
};
}
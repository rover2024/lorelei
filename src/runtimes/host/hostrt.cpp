#include "hostrt_p.h"

#include <map>
#include <string>
#include <shared_mutex>

struct LoreHostRuntimeContext {
    std::map<std::string, std::string> emuValues;
    std::shared_mutex mtx;

    LoreEmuApis emuApi{
        nullptr,
        nullptr,
        nullptr,
        nullptr,
    };
};

static LoreHostRuntimeContext contextInstance;

struct LoreEmuApis *Lore_HRTGetEmuApis() {
    return &contextInstance.emuApi;
}

const char *Lore_HRTGetValue(const char *key) {
    std::shared_lock<std::shared_mutex> lock(contextInstance.mtx);
    auto &map = contextInstance.emuValues;
    auto it = map.find(key);
    if (it == map.end()) {
        return nullptr;
    }
    return it->second.data();
}

bool Lore_HRTSetValue(const char *key, const char *value) {
    std::unique_lock<std::shared_mutex> lock(contextInstance.mtx);
    auto &map = contextInstance.emuValues;
    return map.insert(std::make_pair(key, value)).second;
}
#include "hostrt_p.h"

#ifdef __linux__
#  include <elf.h>
#  include <link.h>
#endif

#include <limits.h>

#include <map>
#include <string>
#include <shared_mutex>
#include <cstring>
#include <fstream>
#include <sstream>
#include <utility>

#include <json11/json11.hpp>

#include "loreuser.h"
#include "util.h"

static const char ENV_LORELEI_LIBRARY_DATA_FILE[] = "LORELEI_LIBRARY_DATA_FILE";

static const char ENV_LORELEI_ROOT[] = "LORELEI_ROOT";

struct LoreGuestLibraryData {
    std::string name;
    std::string path;
    std::string hostThunk;
    std::string host;
    std::vector<std::string> dependencies;

    // C data
    struct LORE_GUEST_LIBRARY_DATA c_data;
    std::vector<const char *> c_data_dependencies;

    void build_c_data() {
        c_data.name = name.data();
        c_data.path = path.data();
        c_data.hostThunk = hostThunk.data();
        c_data.host = host.data();
        c_data_dependencies.reserve(dependencies.size());
        for (const auto &item : std::as_const(dependencies)) {
            c_data_dependencies.push_back(item.data());
        }
    }
};

struct LoreHostLibraryData {
    std::string name;
    std::string fileName;
    std::vector<std::string> guestThunks;

    // C data
    struct LORE_HOST_LIBRARY_DATA c_data;
    std::vector<const char *> c_data_guestThunks;

    void build_c_data() {
        c_data.name = name.data();
        c_data.fileName = fileName.data();
        c_data_guestThunks.reserve(guestThunks.size());
        for (const auto &item : std::as_const(guestThunks)) {
            c_data_guestThunks.push_back(item.data());
        }
    }
};

struct LoreHostRuntimeContext {
    std::map<std::string, std::string> emuValues;
    std::map<std::string, void *> callbackThunks;

    std::shared_mutex mtx;

    std::string rootPath;
    std::string dataFilePath;

    std::map<std::string, LoreGuestLibraryData> guestDataMap;
    std::map<std::string, LoreHostLibraryData> hostDataMap;

    LoreEmuApis emuApis{
        nullptr,
        nullptr,
        nullptr,
        nullptr,
    };

    LoreHostRuntimeContext() {
        auto root = getenv(ENV_LORELEI_ROOT);
        if (!root) {
            fprintf(stderr, "lorehrt: required variable %s not set", ENV_LORELEI_ROOT);
            std::abort();
        }
        rootPath = root;

        auto dataFile = getenv(ENV_LORELEI_LIBRARY_DATA_FILE);
        if (!dataFile) {
            fprintf(stderr, "lorehrt: required variable %s not set", ENV_LORELEI_LIBRARY_DATA_FILE);
            std::abort();
        }
        dataFilePath = dataFile;
        readLibraryDataFile();
    }

    std::string resolvePath(std::string path) const {
        Util::StringReplaceAll(path, "@", rootPath);
        return path;
    }

    void readLibraryDataFile() {
        std::ifstream file(dataFilePath);
        if (!file.is_open()) {
            fprintf(stderr, "lorehrt: %s: open file failed", dataFilePath.c_str());
            std::abort();
        }

        std::stringstream ss;
        ss << file.rdbuf();

        std::string err;
        auto json = json11::Json::parse(ss.str(), err);
        if (!err.empty()) {
            fprintf(stderr, "lorehrt: %s: read file failed: %s", dataFilePath.c_str(), err.c_str());
            std::abort();
        }
        if (!json.is_object()) {
            fprintf(stderr, "lorehrt: %s: not an object", dataFilePath.c_str());
            std::abort();
        }

        // read guest data
        const auto &docObj = json.object_items();
        if (auto it0 = docObj.find("guest"); it0 != docObj.end() && it0->second.is_array()) {
            const auto &guestItems = it0->second.array_items();
            for (const auto &guestItem : guestItems) {
                if (!guestItem.is_object()) {
                    continue;
                }
                const auto &guestItemObj = guestItem.object_items();
                LoreGuestLibraryData result;

                // name
                auto it = guestItemObj.find("name");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.name = val;
                }

                // path
                it = guestItemObj.find("path");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.path = resolvePath(val);
                }

                // hostThunk
                it = guestItemObj.find("hostThunk");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.hostThunk = resolvePath(val);
                }

                // host
                it = guestItemObj.find("host");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.host = resolvePath(val);
                }

                // dependencies
                it = guestItemObj.find("dependencies");
                if (it != guestItemObj.end() && it->second.is_array()) {
                    const auto &depItems = it->second.array_items();
                    for (const auto &depItem : depItems) {
                        if (!depItem.is_string()) {
                            continue;
                        }
                        auto val = depItem.string_value();
                        if (val.empty()) {
                            continue;
                        }
                        result.dependencies.push_back(val);
                    }
                }

                result.build_c_data();
                std::ignore = guestDataMap.insert(std::make_pair(result.name, result));
            }
        }

        // read host data
        if (auto it0 = docObj.find("host"); it0 != docObj.end() && it0->second.is_array()) {
            const auto &hostItems = it0->second.array_items();
            for (const auto &hostItem : hostItems) {
                if (!hostItem.is_object()) {
                    continue;
                }
                const auto &hostItemObj = hostItem.object_items();
                LoreHostLibraryData result;

                // name
                auto it = hostItemObj.find("name");
                if (it == hostItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.name = val;
                }

                // fileName
                it = hostItemObj.find("fileName");
                if (it == hostItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.fileName = val;
                }

                // dependencies
                it = hostItemObj.find("guestThunks");
                if (it != hostItemObj.end() && it->second.is_array()) {
                    const auto &items = it->second.array_items();
                    for (const auto &item : items) {
                        if (!item.is_string()) {
                            continue;
                        }
                        auto val = item.string_value();
                        if (val.empty()) {
                            continue;
                        }
                        result.guestThunks.push_back(val);
                    }
                }

                result.build_c_data();
                std::ignore = hostDataMap.insert(std::make_pair(result.name, result));
            }
        }
    }
};

static LoreHostRuntimeContext contextInstance;

struct LoreEmuApis *Lore_HRTGetEmuApis() {
    return &contextInstance.emuApis;
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

#ifdef __linux__
static unsigned int Lore_AuditObjOpen(struct link_map *map, Lmid_t lmid, uintptr_t *cookie, const char *target) {
    printf("Loading dynamic library: %s %s\n", map->l_name, target);
    return LA_FLG_BINDTO | LA_FLG_BINDFROM;
}

static unsigned int Lore_AuditObjClose(struct link_map *map, uintptr_t *cookie) {
    printf("Closing dynamic library: %s\n", map->l_name);
    return 0;
}

static unsigned int Lore_AuditPreinit(struct link_map *map, uintptr_t *cookie) {
    printf("Init dynamic library: %s\n", map->l_name);
    return 0;
}

static int var_foo = 114514;

static uintptr_t Lore_AuditSymBind64(Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook, uintptr_t *defcook,
                                     unsigned int *flags, const char *symname) {
    printf("Looking up symbol 64: %s, org: %p\n", symname, (void *) sym->st_value);
    if (strcmp(symname, "var_foo") == 0) {
        return (uintptr_t) &var_foo;
    }
    return sym->st_value;
}
#endif

void Lore_HandleExtraGuestCall(int type, void **args, void *ret) {
    switch (type) {
#ifdef __linux__
        case LOREUSER_CT_LA_ObjOpen: {
            *(unsigned int *) ret =
                Lore_AuditObjOpen((link_map *) args[0], (Lmid_t) args[1], (uintptr_t *) args[2], (char *) args[3]);
            break;
        }

        case LOREUSER_CT_LA_ObjClose:
            *(unsigned int *) ret = Lore_AuditObjClose((link_map *) args[0], (uintptr_t *) args[1]);
            break;

        case LOREUSER_CT_LA_PreInit:
            *(unsigned int *) ret = Lore_AuditPreinit((link_map *) args[0], (uintptr_t *) args[1]);
            break;

        case LOREUSER_CT_LA_SymBind:
            *(uintptr_t *) ret =
                Lore_AuditSymBind64((Elf64_Sym *) args[0], (unsigned int) (uintptr_t) args[1], (uintptr_t *) args[2],
                                    (uintptr_t *) args[3], (unsigned int *) args[4], (char *) args[5]);
            break;
#endif
        case LOREUSER_CT_AddCallbackThunk: {
            auto sign = (char *) (args[0]);
            contextInstance.callbackThunks[sign] = args[1];
            break;
        }

        case LOREUSER_CT_GetLibraryData: {
            auto path = (char *) (args[0]);
            bool isGuest = (intptr_t) (args[1]);
            *(void **) ret = Lore_GetLibraryDataH(path, isGuest);
            break;
        }

        case LOREUSER_CT_CallHostHelper: {
            Lore_HostHelper(intptr_t(args[0]), (void **) args[1], args[2]);
            break;
        }

        default:
            break;
    }
}
void *Lore_GetFPExecuteCallback() {
    return (void *) contextInstance.emuApis.ExecuteCallback;
}

void *Lore_GetCallbackThunk(const char *sign) {
    auto it = contextInstance.callbackThunks.find(sign);
    if (it == contextInstance.callbackThunks.end()) {
        return nullptr;
    }
    return it->second;
}

void *Lore_GetLibraryDataH(const char *path, bool isGuest) {
    char nameBuf[PATH_MAX];
    Lore_GetLibraryName(nameBuf, path);

    std::string name(nameBuf);
    if (std::string_view(name).substr(name.size() - 4, 4) == "_HTL") {
        name = name.substr(0, name.size() - 4);
    }

    if (isGuest) {
        auto it = contextInstance.guestDataMap.find(path);
        if (it == contextInstance.guestDataMap.end()) {
            return nullptr;
        }
        return &it->second.c_data;
    }
    auto it = contextInstance.hostDataMap.find(path);
    if (it == contextInstance.hostDataMap.end()) {
        return nullptr;
    }
    return &it->second.c_data;
}
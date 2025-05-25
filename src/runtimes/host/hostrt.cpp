#include "lorehapi_p.h"

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
#include <filesystem>
#include <set>

#include <json11/json11.hpp>

#include "loreuser.h"
#include "lorehapi.h"
#include "util.h"

namespace fs = std::filesystem;

static const char ENV_LORELEI_LIBRARY_DATA_FILE[] = "LORELEI_LIBRARY_DATA_FILE";

static const char ENV_LORELEI_ROOT[] = "LORELEI_ROOT";

static const char LORELEI_ARCH_NAME[] =
#ifdef __aarch64__
    "aarch64"
#elif defined(__riscv)
    "riscv64"
#else
    "x86_64"
#endif
    ;


struct LoreThunkLibraryData {
    std::string name;
    std::vector<std::string> alias;
    std::string gtl;
    std::string htl;
    std::string hl;
    std::vector<std::string> dependencies;

    // C data
    struct LORE_THUNK_LIBRARY_DATA c_data {};
    std::vector<const char *> c_data_dependencies;

    void build_c_data() {
        c_data.name = name.data();
        c_data.gtl = gtl.data();
        c_data.htl = htl.data();
        c_data.hl = hl.data();
        c_data_dependencies.reserve(dependencies.size());
        for (const auto &item : std::as_const(dependencies)) {
            c_data_dependencies.push_back(item.data());
        }
    }
};

struct LoreHostLibraryData {
    std::string name;
    std::vector<std::string> alias;
    std::string fileName;
    std::vector<std::string> thunks;

    // C data
    struct LORE_HOST_LIBRARY_DATA c_data {};
    std::vector<const char *> c_data_thunks;

    void build_c_data() {
        c_data.name = name.data();
        c_data.fileName = fileName.data();
        c_data_thunks.reserve(thunks.size());
        for (const auto &item : std::as_const(thunks)) {
            c_data_thunks.push_back(item.data());
        }
        c_data.thunksCount = c_data_thunks.size();
        c_data.thunks = c_data_thunks.data();
    }
};

struct LoreHostRuntimeContext {
    std::shared_mutex mtx;

    std::filesystem::path rootPath;
    std::filesystem::path dataFilePath;

    std::map<std::string, size_t> thunkNamesMap;
    std::map<std::string, size_t> hostNamesMap;

    std::vector<LoreThunkLibraryData> thunkDataList;
    std::vector<LoreHostLibraryData> hostDataList;

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
        if (!fs::is_directory(root)) {
            fprintf(stderr, "lorehrt: %s: not a directory", root);
            std::abort();
        }
        rootPath = fs::canonical(root);

        auto dataFile = getenv(ENV_LORELEI_LIBRARY_DATA_FILE);
        if (!dataFile) {
            dataFilePath = rootPath / (std::string("etc/lorelei/libs-") + LORELEI_ARCH_NAME + ".json");
        } else {
            dataFilePath = dataFile;
        }
        if (!fs::is_regular_file(dataFilePath)) {
            fprintf(stderr, "lorehrt: %s: not a regular file", dataFilePath.c_str());
            std::abort();
        }
        dataFilePath = fs::canonical(dataFilePath);

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

        // read thunk data
        const auto &docObj = json.object_items();
        if (auto it0 = docObj.find("thunks"); it0 != docObj.end() && it0->second.is_array()) {
            const auto &guestItems = it0->second.array_items();
            for (const auto &guestItem : guestItems) {
                if (!guestItem.is_object()) {
                    continue;
                }
                const auto &guestItemObj = guestItem.object_items();
                LoreThunkLibraryData result;

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

                // alias
                it = guestItemObj.find("alias");
                if (it != guestItemObj.end() && it->second.is_array()) {
                    const auto &aliasItems = it->second.array_items();
                    for (const auto &aliasItem : aliasItems) {
                        if (!aliasItem.is_string()) {
                            continue;
                        }
                        auto val = aliasItem.string_value();
                        if (val.empty()) {
                            continue;
                        }
                        result.alias.push_back(val);
                    }
                }

                // path
                it = guestItemObj.find("gtl");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.gtl = resolvePath(val);
                }

                // hostThunk
                it = guestItemObj.find("htl");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.htl = resolvePath(val);
                }

                // host
                it = guestItemObj.find("hl");
                if (it == guestItemObj.end() || !it->second.is_string()) {
                    continue;
                }
                if (auto val = it->second.string_value(); val.empty()) {
                    continue;
                } else {
                    result.hl = resolvePath(val);
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
                thunkDataList.push_back(result);
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

                // alias
                it = hostItemObj.find("alias");
                if (it != hostItemObj.end() && it->second.is_array()) {
                    const auto &aliasItems = it->second.array_items();
                    for (const auto &aliasItem : aliasItems) {
                        if (!aliasItem.is_string()) {
                            continue;
                        }
                        auto val = aliasItem.string_value();
                        if (val.empty()) {
                            continue;
                        }
                        result.alias.push_back(val);
                    }
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
                it = hostItemObj.find("thunks");
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
                        result.thunks.push_back(val);
                    }
                }

                result.build_c_data();
                hostDataList.push_back(result);
            }
        }

        // Build C data
        for (size_t i = 0; i < thunkDataList.size(); i++) {
            auto &data = thunkDataList[i];
            thunkNamesMap[data.name] = i;
            for (const auto &alias : data.alias) {
                thunkNamesMap[alias] = i;
            }
            data.build_c_data();
        }
        for (size_t i = 0; i < hostDataList.size(); i++) {
            auto &data = hostDataList[i];
            hostNamesMap[data.name] = i;
            for (const auto &alias : data.alias) {
                hostNamesMap[alias] = i;
            }
            data.build_c_data();
        }

        // Build LD_LIBRARY_PATH
        {
            std::vector<std::string> pathList;
            std::set<std::string> pathSet;
            const auto &addPath = [&pathList, &pathSet](const fs::path &path) {
                if (fs::is_directory(path)) {
                    std::string canonicalPath = fs::canonical(path).string();
                    if (!pathSet.count(canonicalPath)) {
                        pathList.push_back(canonicalPath);
                        pathSet.insert(canonicalPath);
                    }
                }
            };

            addPath(rootPath / "lib");

            auto orgPathListStr = getenv("LD_LIBRARY_PATH");
            if (orgPathListStr) {
                for (const auto &item : Util::SplitString(orgPathListStr, ":")) {
                    addPath(fs::path(item));
                }
            }

            for (const auto &item : std::as_const(thunkDataList)) {
                addPath(fs::path(item.hl).parent_path());
            }

            setenv("LD_LIBRARY_PATH", Util::JoinString(pathList, ":").c_str(), true);
        }
    }
};

static LoreHostRuntimeContext LoreHrtCtx;

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
        case LOREUSER_CT_GetLibraryData: {
            auto path = (char *) (args[0]);
            int isGuest = (intptr_t) (args[1]);
            *(void **) ret = Lore_HrtGetLibraryData(path, isGuest);
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

LORELEI_DECL_EXPORT __thread void *Lore_HRTThreadCallback;

struct LoreEmuApis *Lore_HrtGetEmuApis() {
    return &LoreHrtCtx.emuApis;
}

void Lore_HrtSetThreadCallback(void *callback) {
    Lore_HRTThreadCallback = callback;
}

void *Lore_HrtGetLibraryData(const char *path, int isThunk) {
    char nameBuf[PATH_MAX];
    Lore_GetLibraryName(nameBuf, path);

    std::string name(nameBuf);
    if (std::string_view(name).substr(name.size() - 4, 4) == "_HTL") {
        name = name.substr(0, name.size() - 4);
    }

    if (isThunk) {
        auto it = LoreHrtCtx.thunkNamesMap.find(name);
        if (it == LoreHrtCtx.thunkNamesMap.end()) {
            return nullptr;
        }
        return &LoreHrtCtx.thunkDataList[it->second].c_data;
    }

    auto it = LoreHrtCtx.hostNamesMap.find(path);
    if (it == LoreHrtCtx.hostNamesMap.end()) {
        return nullptr;
    }
    return &LoreHrtCtx.hostDataList[it->second].c_data;
}

void *Lore_HrtGetLibraryThunks(const char *path, int isGuest) {
    auto lib_data = (struct LORE_THUNK_LIBRARY_DATA *) Lore_HrtGetLibraryData(path, true);
    if (!lib_data) {
        return nullptr;
    }
    if (isGuest) {
        return lib_data->guestThunks;
    }
    return lib_data->hostThunks;
}

void *Lore_LoadHostLibrary(void *someAddr, int thunkCount, void **thunks) {
    char buf[PATH_MAX];
    if (!Lore_RevealLibraryPath(buf, someAddr, true)) {
        fprintf(stderr, "lorehrt: %p: failed to reveal library path\n", someAddr);
        return nullptr;
    }

    auto lib_data = (struct LORE_THUNK_LIBRARY_DATA *) Lore_HrtGetLibraryData(buf, true);
    if (!lib_data) {
        fprintf(stderr, "lorehrt: %s: failed to get library data\n", buf);
        return nullptr;
    }

    // Load host
    auto handle = dlopen(lib_data->hl, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "lorehrt: %s: failed to load host library \"%s\": %s\n", buf, lib_data->hl, dlerror());
        return nullptr;
    }

    lib_data->hostThunkCount = thunkCount;
    lib_data->hostThunks = thunks;
    return handle;
}

void Lore_FreeHostLibrary(void *handle) {
    dlclose(handle);
}

int Lore_HrtPThreadCreate(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg) {
    int ret;
    LoreHrtCtx.emuApis.NotifyPThreadCreate(thread, attr, start_routine, arg, &ret);
    *thread = LoreHrtCtx.emuApis.GetLastPThreadId();

    auto self = pthread_self();
    auto last = LoreHrtCtx.emuApis.GetLastPThreadId();
    return ret;
}

void Lore_HrtPThreadExit(void *ret) {
    LoreHrtCtx.emuApis.NotifyPThreadExit(ret);
    __builtin_unreachable();
}
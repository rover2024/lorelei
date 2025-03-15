#include "hostrt_p.h"

#include <elf.h>
#include <link.h>

#include <map>
#include <string>
#include <shared_mutex>
#include <cstring>

#include "loreuser.h"

static const char ENV_LORELEI_LIBRARY_DATA_FILE[] = "LORELEI_LIBRARY_DATA_FILE";

static const char ENV_LORELEI_ROOT[] = "LORELEI_ROOT";

struct LoreHostRuntimeContext {
    std::map<std::string, std::string> emuValues;
    std::map<std::string, void *> callbackThunks;

    std::shared_mutex mtx;

    std::string rootPath;
    std::string dataFilePath;

    LoreEmuApis emuApi{
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
            
        }
    }
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

static uintptr_t host_audit_symbind64(Elf64_Sym *sym, unsigned int ndx, uintptr_t *refcook, uintptr_t *defcook,
                                      unsigned int *flags, const char *symname) {
    printf("Looking up symbol 64: %s, org: %p\n", symname, (void *) sym->st_value);
    if (strcmp(symname, "var_foo") == 0) {
        return (uintptr_t) &var_foo;
    }
    return sym->st_value;
}

void Lore_HandleExtraGuestCall(int type, void *args[]) {
    switch (type) {
        case LOREUSER_CT_LA_ObjOpen: {
            *(unsigned int *) args[4] =
                Lore_AuditObjOpen((link_map *) args[0], (Lmid_t) args[1], (uintptr_t *) args[2], (char *) args[3]);
            break;
        }

        case LOREUSER_CT_LA_ObjClose:
            *(unsigned int *) args[2] = Lore_AuditObjClose((link_map *) args[0], (uintptr_t *) args[1]);
            break;

        case LOREUSER_CT_LA_PreInit:
            *(unsigned int *) args[2] = Lore_AuditPreinit((link_map *) args[0], (uintptr_t *) args[1]);
            break;

        case LOREUSER_CT_LA_SymBind:
            *(uintptr_t *) args[6] =
                host_audit_symbind64((Elf64_Sym *) args[0], (unsigned int) (uintptr_t) args[1], (uintptr_t *) args[2],
                                     (uintptr_t *) args[3], (unsigned int *) args[4], (char *) args[5]);
            break;

        case LOREUSER_CT_AddCallbackThunk: {
            auto sign = (char *) (args[0]);
            contextInstance.callbackThunks[sign] = args[1];
            break;
        }

            // case LOREUSER_CT_SearchLibrary: {
            //     auto path = (char *) (args[0]);
            //     auto mode = (int) (uintptr_t) args[1];
            //     auto ret_ref = (const char **) args[2];
            //     *ret_ref = x64nc_SearchLibraryH(path, mode);
            //     break;
            // }

        default:
            break;
    }
}
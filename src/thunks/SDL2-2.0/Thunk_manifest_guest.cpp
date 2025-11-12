#include "Thunk_procs_desc.h"
#include <lorelei/TLCMeta/ManifestContext_guest.inc.h>

// #define LORETHUNK_CALLBACK_REPLACE

#include <dlfcn.h>

namespace lorethunk {

    template <>
    struct MetaProc<::SDL_LoadObject, _HFN, _GTP> {
        _PROC void *invoke(const char *sofile) {
            return dlopen(sofile, RTLD_NOW);
        }
    };

    template <>
    struct MetaProc<::SDL_LoadFunction, _HFN, _GTP> {
        _PROC void *invoke(void *handle, const char *name) {
            return dlsym(handle, name);
        }
    };

    template <>
    struct MetaProc<::SDL_UnloadObject, _HFN, _GTP> {
        _PROC int invoke(void *handle) {
            return dlclose(handle);
        }
    };

}
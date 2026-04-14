#include "Desc.h"
#include <lorelei/Tools/ThunkInterface/Guest/ManifestDef.cpp.inc>

// #define LORE_THUNK_CALLBACK_REPLACE

#include <dlfcn.h>

namespace lore::thunk {

    template <>
    struct ProcFn<::SDL_LoadObject, GuestToHost, Entry> {
        static void *invoke(const char *sofile) {
            return dlopen(sofile, RTLD_NOW);
        }
    };

    template <>
    struct ProcFn<::SDL_LoadFunction, GuestToHost, Entry> {
        static void *invoke(void *handle, const char *name) {
            return dlsym(handle, name);
        }
    };

    template <>
    struct ProcFn<::SDL_UnloadObject, GuestToHost, Entry> {
        static int invoke(void *handle) {
            return dlclose(handle);
        }
    };

}

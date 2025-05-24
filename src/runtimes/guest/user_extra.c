#include "loreuser.h"

#define _GNU_SOURCE

#include <dlfcn.h>
#include <limits.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "loreshared.h"

void *Lore_LoadHostThunkLibrary(void *someAddr, int thunkCount, void **thunks) {
    char buf[PATH_MAX];
    if (!Lore_RevealLibraryPath(buf, someAddr, true)) {
        return NULL;
    }

    struct LORE_THUNK_LIBRARY_DATA *lib_data = Lore_GetLibraryData(buf, true);
    if (!lib_data) {
        return NULL;
    }

    // Load host thunk
    void *host_thunk_handle = Lore_LoadLibrary(lib_data->htl, RTLD_NOW);
    if (!host_thunk_handle) {
        return NULL;
    }

    // Load dependencies
    const char **deps = lib_data->dependencies;
    for (int i = 0; i < lib_data->dependencyCount; ++i) {
        struct LORE_THUNK_LIBRARY_DATA *dep_lib_data = Lore_GetLibraryData(deps[i], true);
        if (!dep_lib_data) {
            fprintf(stderr, "loregrt: failed to get data of dependency \"%s\"\n", deps[i]);
            continue;
        }
        const char *path = dep_lib_data->gtl;
        void *guest_handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        if (!guest_handle) {
            guest_handle = dlopen(path, RTLD_NOW);
            if (!guest_handle) {
                fprintf(stderr, "loregrt: failed to load dependency \"%s\"\n", path);
                continue;
            }
        } else {
            // https://stackoverflow.com/questions/50570223/dlopenrtld-noload-still-returns-not-null-after-dlclose
            dlclose(guest_handle);
        }
    }

    lib_data->guestThunkCount = thunkCount;
    lib_data->guestThunks = thunks;
    return host_thunk_handle;
}

void Lore_FreeHostThunkLibrary(void *handle) {
    do {
        const char *host_thunk_path = Lore_GetModulePath(handle, true);
        if (!host_thunk_path) {
            break;
        }

        struct LORE_THUNK_LIBRARY_DATA *lib_data = Lore_GetLibraryData(host_thunk_path, true);
        if (!lib_data) {
            break;
        }

        // Free dependencies
        const char **deps = lib_data->dependencies;
        for (int i = 0; i < lib_data->dependencyCount; ++i) {
            struct LORE_THUNK_LIBRARY_DATA *dep_lib_data = Lore_GetLibraryData(deps[i], true);
            if (!dep_lib_data) {
                continue;
            }
            const char *path = dep_lib_data->gtl;
            void *guest_handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
            if (guest_handle) {
                dlclose(guest_handle);
                dlclose(guest_handle);
            }
        }
    } while (false);

    // Free host thunk
    Lore_FreeHostThunkLibrary(handle);
}

void *Lore_GetHostThunkProcAddress(void *handle, const char *name) {
    char realName[1024];
    strcpy(realName, "_HTP_");
    strcat(realName, name);
    return Lore_GetProcAddress(handle, realName);
}

void *Lore_ConvertHostProcAddress(const char *name, void *addr) {
    const char *host_library_path = Lore_GetModulePath(addr, false);
    if (!host_library_path) {
        return NULL;
    }

    struct LORE_HOST_LIBRARY_DATA *lib_data = Lore_GetLibraryData(host_library_path, false);
    if (!lib_data) {
        return NULL;
    }

    const char **thunks = lib_data->thunks;
    for (int i = 0; i < lib_data->thunksCount; ++i) {
        const char *thunkName = thunks[i];
        struct LORE_THUNK_LIBRARY_DATA *thunk_lib_data = Lore_GetLibraryData(thunkName, true);
        if (!thunk_lib_data) {
            continue;
        }
        const char *path = thunk_lib_data->gtl;
        void *guest_handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        if (!guest_handle) {
            guest_handle = dlopen(path, RTLD_NOW);
            if (!guest_handle) {
                continue;
            }
        }
        void *func = dlsym(guest_handle, name);
        if (!func) {
            continue;
        }
        return func;
    }
    return NULL;
}
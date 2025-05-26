#include "loreuser.h"

#define _GNU_SOURCE

#include <dlfcn.h>
#include <limits.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "loreshared.h"

void *Lore_LoadHostThunkLibrary(void *someAddr) {
    char buf[PATH_MAX];
    if (!Lore_RevealLibraryPath(buf, someAddr, 1)) {
        fprintf(stderr, "loregrt: %p: failed to reveal library path\n", someAddr);
        return NULL;
    }

    struct LORE_THUNK_LIBRARY_DATA *lib_data = Lore_GetLibraryData(buf, 1);
    if (!lib_data) {
        fprintf(stderr, "loregrt: %s: failed to get library data\n", buf);
        return NULL;
    }

    // Load dependencies
    const char **deps = lib_data->dependencies;
    for (int i = 0; i < lib_data->dependencyCount; ++i) {
        struct LORE_THUNK_LIBRARY_DATA *dep_lib_data = Lore_GetLibraryData(deps[i], 1);
        if (!dep_lib_data) {
            fprintf(stderr, "loregrt: %s: failed to get data of dependency \"%s\"\n", buf, deps[i]);
            continue;
        }
        const char *path = dep_lib_data->gtl;
        void *guest_handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        if (!guest_handle) {
            guest_handle = dlopen(path, RTLD_NOW);
            if (!guest_handle) {
                fprintf(stderr, "loregrt: %s: failed to load dependency \"%s\"\n", buf, path);
                continue;
            }
        } else {
            // https://stackoverflow.com/questions/50570223/dlopenrtld-noload-still-returns-not-null-after-dlclose
            dlclose(guest_handle);
        }
    }

    // Load host thunk
    void *host_thunk_handle = Lore_LoadLibrary(lib_data->htl, RTLD_NOW);
    if (!host_thunk_handle) {
        fprintf(stderr, "loregrt: %s: failed to load HTL \"%s\": %s\n", buf, lib_data->htl, Lore_GetErrorMessage());
        return NULL;
    }

    return host_thunk_handle;
}

void Lore_FreeHostThunkLibrary(void *handle) {
    const char *host_thunk_path = Lore_GetModulePath(handle, 1);

    // Free host thunk
    Lore_FreeLibrary(handle);

    if (host_thunk_path) {
        struct LORE_THUNK_LIBRARY_DATA *lib_data = Lore_GetLibraryData(host_thunk_path, 1);
        if (lib_data) {
            // Free dependencies
            const char **deps = lib_data->dependencies;
            for (int i = lib_data->dependencyCount - 1; i >= 0; --i) {
                struct LORE_THUNK_LIBRARY_DATA *dep_lib_data = Lore_GetLibraryData(deps[i], 1);
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
        }
    }
}

void *Lore_GetHostThunkProcAddress(void *handle, const char *name) {
    char realName[1024];
    strcpy(realName, "_HTP_");
    strcat(realName, name);
    return Lore_GetProcAddress(handle, realName);
}

void *Lore_ConvertHostProcAddress(const char *name, void *addr) {
    const char *host_library_path = Lore_GetModulePath(addr, 0);
    if (!host_library_path) {
        fprintf(stderr, "loregrt: %p: failed to reveal library path\n", addr);
        return NULL;
    }

    struct LORE_HOST_LIBRARY_DATA *lib_data = Lore_GetLibraryData(host_library_path, 0);
    if (!lib_data) {
        fprintf(stderr, "loregrt: %s: failed to get library data\n", host_library_path);
        return NULL;
    }

    const char **thunks = lib_data->thunks;
    for (int i = 0; i < lib_data->thunksCount; ++i) {
        const char *thunkName = thunks[i];
        struct LORE_THUNK_LIBRARY_DATA *thunk_lib_data = Lore_GetLibraryData(thunkName, 1);
        if (!thunk_lib_data) {
            fprintf(stderr, "loregrt: %s: failed to get data of thunk library \"%s\"\n", host_library_path, thunkName);
            continue;
        }
        const char *path = thunk_lib_data->gtl;
        void *guest_handle = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        if (!guest_handle) {
            guest_handle = dlopen(path, RTLD_NOW);
            if (!guest_handle) {
                fprintf(stderr, "loregrt: %s: failed to load thunk library \"%s\"\n", host_library_path, path);
                continue;
            }
        }
        void *func = dlsym(guest_handle, name);
        if (!func) {
            continue;
        }
        return func;
    }
    fprintf(stderr, "loregrt: %s: failed to convert host function \"%s\" at %p to guest function\n", host_library_path,
            name, addr);
    return NULL;
}
#include "loreuser.h"

#include <stddef.h>
#include <stdint.h>

#include "syscall_helper.h"

void *Lore_LoadLibrary(const char *path, int flags) {
    void *ret = NULL;
    void *a[] = {
        (char *) (path),
        (void *) (uintptr_t) flags,
        &ret,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_LoadLibrary, a);
    return ret;
}

int Lore_FreeLibrary(void *handle) {
    int ret;
    void *a[] = {
        handle,
        &ret,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_FreeLibrary, a);
    return ret;
}

void *Lore_GetProcAddress(void *handle, const char *name) {
    void *ret = NULL;
    void *a[] = {
        handle,
        (char *) (name),
        &ret,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetProcAddress, a);
    return ret;
}

char *Lore_GetErrorMessage() {
    char *ret = NULL;
    void *a[] = {
        &ret,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetErrorMessage, a);
    return ret;
}

char *Lore_GetModulePath(void *addr, bool isHandle) {
    char *ret = NULL;
    void *a[] = {
        addr,
        (void *) (intptr_t) isHandle,
        &ret,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetModulePath, a);
    return ret;
}

void Lore_AddCallbackThunk(const char *sign, void *func) {
    void *a[] = {
        (char *) (sign),
        func,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_AddCallbackThunk, a);
}

struct LORE_GUEST_LIBRARY_DATA *Lore_GetGuestLibraryData(const char *path, int libType) {
    struct LORE_GUEST_LIBRARY_DATA *ret = NULL;
    void *a[] = {
        (char *) (path),
        (void *) (intptr_t) libType,
        &ret,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetGuestLibraryData, a);
    return ret;
}
#include "loreuser.h"

#include <stddef.h>
#include <stdint.h>

#include "syscall_helper.h"

void *Lore_LoadLibrary(const char *path, int flags) {
    void *ret = NULL;
    void *a[] = {
        (char *) (path),
        (void *) (uintptr_t) flags,
    };
    (void) syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_LoadLibrary, a, &ret);
    return ret;
}

int Lore_FreeLibrary(void *handle) {
    int ret;
    void *a[] = {
        handle,
    };
    (void) syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_FreeLibrary, a, &ret);
    return ret;
}

void *Lore_GetProcAddress(void *handle, const char *name) {
    void *ret = NULL;
    void *a[] = {
        handle,
        (char *) (name),
    };
    (void) syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetProcAddress, a, &ret);
    return ret;
}

char *Lore_GetErrorMessage() {
    char *ret = NULL;
    (void) syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetErrorMessage, NULL, &ret);
    return ret;
}

char *Lore_GetModulePath(void *addr, bool isHandle) {
    char *ret = NULL;
    void *a[] = {
        addr,
        (void *) (intptr_t) isHandle,
    };
    (void) syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetModulePath, a, &ret);
    return ret;
}

void *Lore_GetAddressBoundary() {
    char *ret = NULL;
    (void) syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetAddressBoundary, NULL, &ret);
    return ret;
}

void *Lore_GetLibraryData(const char *path, bool isThunk) {
    void *ret = NULL;
    void *a[] = {
        (char *) (path),
        (void *) (intptr_t) isThunk,
    };
    (void) syscall3(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_GetLibraryData, a, &ret);
    return ret;
}

void Lore_CallHostHelper(int id, void **args, void *ret) {
    void *a[] = {
        (void *) (intptr_t) (id),
        args,
        ret,
    };
    (void) syscall2(LOREUSER_SYSCALL_NUM, (void *) LOREUSER_CT_CallHostHelper, a);
}
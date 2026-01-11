#include "GuestSyscallClient.h"

namespace lore::mod {

    GuestSyscallClient::GuestSyscallClient() = default;

    GuestSyscallClient::~GuestSyscallClient() = default;

    LString GuestSyscallClient::getHostAttribute_impl(const char *key) {
        return {};
    }

    void GuestSyscallClient::logMessage_impl(int level, const LogContext &context,
                                             const char *msg) {
    }

    void *GuestSyscallClient::loadLibrary_impl(const char *path, int flags) {
        return {};
    }

    int GuestSyscallClient::freeLibrary_impl(void *handle) {
        return {};
    }

    void *GuestSyscallClient::getProcAddress_impl(void *handle, const char *name) {
        return {};
    }

    char *GuestSyscallClient::getErrorMessage_impl() {
        return {};
    }

    char *GuestSyscallClient::getModulePath_impl(void *opaque, bool isHandle) {
        return {};
    }

    int GuestSyscallClient::invokeProc_impl(void *proc, int conv, void *opaque) {
        return {};
    }

    LThunkInfo GuestSyscallClient::getThunkInfo_impl(const char *path, bool isReverse) {
        return {};
    }

    void *GuestSyscallClient::heapAlloc_impl(size_t size) {
        return {};
    }

    void GuestSyscallClient::heapFree_impl(void *ptr) {
    }

    void GuestSyscallClient::setSpecialEntry_impl(int id, void *addr) {
    }


}

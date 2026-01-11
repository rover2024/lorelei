#ifndef LORE_MODULES_GUESTRT_GUESTSYSCALLCLIENT_H
#define LORE_MODULES_GUESTRT_GUESTSYSCALLCLIENT_H

#include <LoreBase/RuntimeBase/PassThrough/IClient.h>
#include <LoreMods/GuestRT/Global.h>

namespace lore::mod {

    class LOREGUESTRT_EXPORT GuestSyscallClient : public IClient<GuestSyscallClient> {
    public:
        GuestSyscallClient();
        ~GuestSyscallClient();

    protected:
        static LString getHostAttribute_impl(const char *key);
        static void logMessage_impl(int level, const LogContext &context, const char *msg);
        static void *loadLibrary_impl(const char *path, int flags);
        static int freeLibrary_impl(void *handle);
        static void *getProcAddress_impl(void *handle, const char *name);
        static char *getErrorMessage_impl();
        static char *getModulePath_impl(void *opaque, bool isHandle);
        static int invokeProc_impl(void *proc, int conv, void *opaque);
        static LThunkInfo getThunkInfo_impl(const char *path, bool isReverse);
        static void *heapAlloc_impl(size_t size);
        static void heapFree_impl(void *ptr);
        static void setSpecialEntry_impl(int id, void *addr);

        friend class IClient<GuestSyscallClient>;
    };

}

#endif // LORE_MODULES_GUESTRT_GUESTSYSCALLCLIENT_H

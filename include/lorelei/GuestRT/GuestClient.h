#ifndef LOREGUESTRT_GUESTSYSCALLBRIDGE_H
#define LOREGUESTRT_GUESTSYSCALLBRIDGE_H

#include <lorelei/Core/Connect/SyscallClient.h>
#include <lorelei/GuestRT/Global.h>

namespace lore {

    class LOREGUESTRT_EXPORT GuestClient : public SyscallClient<GuestClient> {
    public:
        GuestClient();
        ~GuestClient();

        static GuestClient *instance();

    public:
        void *convertHostProcAddress(const char *name, void *addr);
    
    protected:
        /// Implementation of \c Client
        int checkConnection_impl();
        void logMessage_impl(int level, const void *context, const char *msg);

        void *loadLibrary_impl(const char *path, int flags);
        int freeLibrary_impl(void *handle);
        void *getProcAddress_impl(void *handle, const char *name);
        char *getErrorMessage_impl();
        char *getModulePath_impl(void *opaque, bool isHandle);
        int invokeProc_impl(void *proc, int conv, void *opaque);
        ThunkInfo getThunkInfo_impl(const char *path, bool isReverse);

        friend class Client<GuestClient>;
        friend class SyscallClient<GuestClient>;
    };

}

#endif // LOREGUESTRT_GUESTSYSCALLBRIDGE_H

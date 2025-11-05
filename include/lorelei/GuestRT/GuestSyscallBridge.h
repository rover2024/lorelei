#ifndef LOREGUESTRT_GUESTSYSCALLBRIDGE_H
#define LOREGUESTRT_GUESTSYSCALLBRIDGE_H

#include <lorelei/Core/Bridge/SyscallBridge.h>
#include <lorelei/GuestRT/Global.h>

namespace lore {

    class LOREGUESTRT_EXPORT GuestSyscallBridge : public SyscallBridge<GuestSyscallBridge> {
    public:
        GuestSyscallBridge();
        ~GuestSyscallBridge();

        static GuestSyscallBridge *instance();

    protected:
        /// Implementation of \c Bridge
        int checkHealth_impl();
        void logMessage_impl(int level, const void *context, const char *msg);

        void *loadLibrary_impl(const char *path, int flags);
        int freeLibrary_impl(void *handle);
        void *getProcAddress_impl(void *handle, const char *name);
        char *getErrorMessage_impl();
        char *getModulePath_impl(void *opaque, bool isHandle);
        int invokeProc_impl(void *proc, int conv, void *opaque);
        ThunkInfo getThunkInfo_impl(const char *path, bool isReverse);

        friend class Bridge<GuestSyscallBridge>;
        friend class SyscallBridge<GuestSyscallBridge>;
    };

}

#endif // LOREGUESTRT_GUESTSYSCALLBRIDGE_H

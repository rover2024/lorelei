#ifndef LOREGUESTRT_GUESTTHUNKCONTEXT_H
#define LOREGUESTRT_GUESTTHUNKCONTEXT_H

#include <lorelei-c/Core/ProcInfo_c.h>
#include <lorelei/GuestRT/Global.h>

namespace lore {

    class LOREGUESTRT_EXPORT GuestThunkContext {
    public:
        inline GuestThunkContext(const char *moduleName, CStaticProcInfoContext *ctx)
            : _moduleName(moduleName), _procInfoCtx(ctx) {
            initThunks();
        }

        ~GuestThunkContext();

        const char *moduleName() const {
            return _moduleName;
        }

        const CStaticProcInfoContext *procInfoContext() const {
            return _procInfoCtx;
        }

    protected:
        const char *_moduleName;
        CStaticProcInfoContext *_procInfoCtx;

        // Context data
        void *_handle = nullptr;

        void initThunks();
    };

}

#endif // LOREGUESTRT_GUESTTHUNKCONTEXT_H

#ifndef LOREHOSTRT_HOSTTHUNKCONTEXT_H
#define LOREHOSTRT_HOSTTHUNKCONTEXT_H

#include <lorelei-c/Core/ProcInfo_c.h>
#include <lorelei/HostRT/Global.h>

namespace lore {

    class LOREHOSTRT_EXPORT HostThunkContext {
    public:
        inline HostThunkContext(const char *moduleName, CStaticProcInfoContext *ctx)
            : _moduleName(moduleName), _procInfoCtx(ctx) {
        }

        ~HostThunkContext();

        const char *moduleName() const {
            return _moduleName;
        }

        const CStaticProcInfoContext *procInfoContext() const {
            return _procInfoCtx;
        }

        void initialize();

    protected:
        const char *_moduleName;
        CStaticProcInfoContext *_procInfoCtx;

        // Context data
        void *_handle = nullptr;
    };

}

#endif // LOREHOSTRT_HOSTTHUNKCONTEXT_H

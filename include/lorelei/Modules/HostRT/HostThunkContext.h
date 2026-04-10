#ifndef LORE_MODULES_HOSTRT_HOSTTHUNKCONTEXT_H
#define LORE_MODULES_HOSTRT_HOSTTHUNKCONTEXT_H

#include <lorelei/Base/PassThrough/c/CProc.h>

#include <lorelei/Modules/HostRT/Global.h>

namespace lore::mod {

    class LOREHOSTRT_EXPORT HostThunkContext {
    public:
        inline HostThunkContext(CStaticThunkContext *localContext)
            : m_staticContext(localContext) {
        }
        ~HostThunkContext();

        inline const CStaticThunkContext *staticThunkContext() const {
            return m_staticContext;
        }

        void initialize();

    protected:
        CStaticThunkContext *m_staticContext;
        void *m_hostLibraryHandle = nullptr;
    };

}

#endif // LORE_MODULES_HOSTRT_HOSTTHUNKCONTEXT_H

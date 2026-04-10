#ifndef LORE_MODULES_GUESTRT_GUESTTHUNKCONTEXT_H
#define LORE_MODULES_GUESTRT_GUESTTHUNKCONTEXT_H

#include <vector>

#include <lorelei/Base/PassThrough/c/CProc.h>
#include <lorelei/Base/PassThrough/ThunkTools/FunctionTrampoline.h>

#include <lorelei/Modules/GuestRT/Global.h>

namespace lore::mod {

    class LOREGUESTRT_EXPORT GuestThunkContext {
    public:
        inline GuestThunkContext(CStaticThunkContext *localContext)
            : m_staticContext(localContext) {
        }
        ~GuestThunkContext();

        inline const CStaticThunkContext *staticThunkContext() const {
            return m_staticContext;
        }

        void initialize();

    protected:
        CStaticThunkContext *m_staticContext;
        void *m_htlHandle = nullptr;
        std::vector<FunctionTrampolineTable *> m_fdgTrampolineTables;
    };

}

#endif // LORE_MODULES_GUESTRT_GUESTTHUNKCONTEXT_H

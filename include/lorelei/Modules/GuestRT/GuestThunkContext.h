// SPDX-License-Identifier: MIT

#ifndef LORE_MODULES_GUESTRT_GUESTTHUNKCONTEXT_H
#define LORE_MODULES_GUESTRT_GUESTTHUNKCONTEXT_H

#include <vector>

#include <lorelei/DLCall/ProcDefs.h>
#include <lorelei/Modules/GuestRT/Global.h>

namespace lore::mod {

    /// GuestThunkContext - The guest runtime's per-thunk-library context.
    ///
    /// Holds the library's \c StaticThunkContext and the loaded host-thunk (HTL) handle.
    /// \c initialize brings the host thunk up so the guest's generated procs can call across the
    /// boundary.
    class LOREGUESTRT_EXPORT GuestThunkContext {
    public:
        inline GuestThunkContext(thunk::StaticThunkContext *localContext)
            : m_staticThunkContext(localContext) {
        }
        ~GuestThunkContext();

        inline const thunk::StaticThunkContext *staticThunkContext() const {
            return m_staticThunkContext;
        }

        void initialize();

    protected:
        thunk::StaticThunkContext *m_staticThunkContext;
        void *m_htlHandle = nullptr;
    };

}

#endif // LORE_MODULES_GUESTRT_GUESTTHUNKCONTEXT_H

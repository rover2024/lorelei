// SPDX-License-Identifier: MIT

#ifndef LORE_MODULES_HOSTRT_HOSTTHUNKCONTEXT_H
#define LORE_MODULES_HOSTRT_HOSTTHUNKCONTEXT_H

#include <lorelei/DLCall/ProcDefs.h>
#include <lorelei/Modules/HostRT/Global.h>

namespace lore::mod {

    /// HostThunkContext - The host runtime's per-thunk-library context.
    ///
    /// Holds the library's \c StaticThunkContext and the real host library handle. \c initialize
    /// resolves the library so the host-side procs can invoke it.
    class LOREHOSTRT_EXPORT HostThunkContext {
    public:
        inline HostThunkContext(thunk::StaticThunkContext *localContext)
            : m_staticThunkContext(localContext) {
        }
        ~HostThunkContext();

        inline const thunk::StaticThunkContext *staticThunkContext() const {
            return m_staticThunkContext;
        }

        void initialize();

    protected:
        thunk::StaticThunkContext *m_staticThunkContext;
        void *m_hostLibraryHandle = nullptr;
    };

}

#endif // LORE_MODULES_HOSTRT_HOSTTHUNKCONTEXT_H

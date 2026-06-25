namespace lore::thunk {

    // Declared in "Callback.h"
    inline bool isHostAddress(void *addr) {
        return reinterpret_cast<uintptr_t>(addr) >
               reinterpret_cast<uintptr_t>(detail::staticThunkContext.emuAddr);
    }

}

#include "ProcImpl.cpp.inc"

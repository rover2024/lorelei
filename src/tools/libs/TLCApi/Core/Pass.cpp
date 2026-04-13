#include "Pass.h"

#include <array>
#include <vector>

namespace llvm {

    template <>
    LORETLCAPI_EXPORT Registry<lore::tool::TLC::Pass>::node *
        Registry<lore::tool::TLC::Pass>::Head = nullptr;

    template <>
    LORETLCAPI_EXPORT Registry<lore::tool::TLC::Pass>::node *
        Registry<lore::tool::TLC::Pass>::Tail = nullptr;

    template class Registry<lore::tool::TLC::Pass>;

}


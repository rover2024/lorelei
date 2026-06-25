#include "Pass.h"

#include <array>
#include <vector>

namespace llvm {

    template <>
    LORETLCAPI_EXPORT Registry<lore::tool::TLC::Pass>::node *Registry<lore::tool::TLC::Pass>::Head =
        nullptr;

    template <>
    LORETLCAPI_EXPORT Registry<lore::tool::TLC::Pass>::node *Registry<lore::tool::TLC::Pass>::Tail =
        nullptr;

    template class Registry<lore::tool::TLC::Pass>;

}

namespace lore::tool::TLC {

    bool Pass::testProc(ProcSnippet &proc, std::unique_ptr<PassMessage> &msg) {
        // if (m_phase == Builder) {
        //     if (auto &desc = proc.desc(); desc) {
        //         if (desc->builderPass.has_value()) {
        //             return desc->builderPass->id == m_id;
        //         }
        //     }
        //     return false;
        // } else {
        //     if (auto &desc = proc.desc(); desc) {
        //         return std::find_if(desc->passes.begin(), desc->passes.end(),
        //                             [this](const auto &pass) { return pass.id == m_id; }) !=
        //                desc->passes.end();
        //     }
        // }
        return false;
    }

}

#include <lorelei/Tools/TLCApi/Core/Pass.h>

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

namespace lore::tool::TLC {

    std::map<int, Pass *> &Pass::passMap(Phase phase) {
        struct RegistryCache {
            std::array<std::map<int, Pass *>, 3> maps;
            std::vector<std::unique_ptr<Pass>> instances;
            bool initialized = false;
        };

        static RegistryCache cache;
        if (!cache.initialized) {
            for (const auto &entry : PassRegistry::entries()) {
                auto pass = entry.instantiate();
                if (!pass) {
                    continue;
                }

                auto phaseIndex = static_cast<size_t>(pass->phase());
                if (phaseIndex >= cache.maps.size()) {
                    continue;
                }

                cache.maps[phaseIndex][pass->id()] = pass.get();
                cache.instances.push_back(std::move(pass));
            }
            cache.initialized = true;
        }

        return cache.maps[phase];
    }

}

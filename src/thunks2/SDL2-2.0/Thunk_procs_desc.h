#include <SDL2/SDL.h>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_User> {
        static constexpr const char moduleName[] = "SDL2-2.0";
    };

    template <>
    struct MetaProcDesc<::SDL_LogMessageV> {
        using builder_pass = MetaPass_vprintf<>;
    };

    using PFN_SDL_LogMessageV = decltype(&::SDL_LogMessageV);

    template <>
    struct MetaProcCBDesc<PFN_SDL_LogMessageV> {
        static constexpr const char name[] = "PFN_SDL_LogMessageV";
        using builder_pass = MetaPass_vprintf<>;
    };

}
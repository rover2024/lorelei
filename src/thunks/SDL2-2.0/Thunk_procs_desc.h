#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <lorelei/TLCMeta/MetaConfig.h>
#include <lorelei/TLCMeta/MetaProcDesc.h>
#include <lorelei/TLCMeta/MetaPass.h>

namespace lorethunk {

    template <>
    struct MetaConfig<MCS_User> {
        _DESC char moduleName[] = "libSDL2-2.0";
    };

    template <>
    struct MetaProcDesc<::SDL_LogMessageV> {
        _DESC MetaPass_vprintf<> builder_pass = {};
    };

}
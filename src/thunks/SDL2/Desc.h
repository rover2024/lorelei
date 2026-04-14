#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <lorelei/Tools/ThunkInterface/Proc.h>
#include <lorelei/Tools/ThunkInterface/PassTags.h>

#ifdef Success
#undef Success
#endif

namespace lore::thunk {

    template <>
    struct ProcFnDesc<::SDL_LogMessageV> {
        _DESC pass::printf<> builder_pass = {};
    };

}

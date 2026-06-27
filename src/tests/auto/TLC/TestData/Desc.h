#pragma once

#include "ThunkExample.h"

#include <lorelei/ThunkInterface/Proc.h>
#include <lorelei/ThunkInterface/PassTags.h>

namespace lore::thunk {

    // Most of the format functions need no descriptor:
    //   - le_printf / le_vprintf / le_sscanf / le_vsscanf are recognised by their names ending in
    //     "printf" / "scanf";
    //   - le_emit_attr / le_vemit_attr are recognised by their printf format attribute, even though
    //     their names do not reveal them.
    //
    // Only the two functions that have neither a telling name nor a format attribute need a
    // descriptor. The pass parameters are <FormatIndex, VariadicIndex>, counted from 1: the format
    // string is the 2nd parameter and the variadic part (or the va_list) is the 3rd. le_emit is the
    // `...` form (printf); le_vemit is the va_list form (vprintf).
    template <>
    struct ProcFnDesc<::le_emit> {
        _DESC pass::printf<2, 3> builder_pass = {};
    };

    template <>
    struct ProcFnDesc<::le_vemit> {
        _DESC pass::vprintf<2, 3> builder_pass = {};
    };

}

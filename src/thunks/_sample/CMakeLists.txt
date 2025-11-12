project(_sample)

if(LORELEI_BUILD_TOOLS AND LORELEI_THUNK_GENERATE)
    add_subdirectory(plugin)
endif()

include("../ThunkAPI.cmake")

set(PLUGIN_target _sample_TLCPlugin)
set(GTL_alias lib_sample.so.0)

add_thunk()
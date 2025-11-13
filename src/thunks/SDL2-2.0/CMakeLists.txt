project(SDL2-2.0)

include("../ThunkAPI.cmake")

set(ALL_extra_includes
    "${LORELEI_HOST_LIBRARIES_DIR}/SDL/build/install/include"
    "${LORELEI_HOST_LIBRARIES_DIR}/SDL/build/install/include/SDL2"
)
set(GTL_alias libSDL2.so libSDL2-2.0.so.0)
set(GTL_extra_links dl)
set(GTL_extra_options -Wno-deprecated-declarations)
set(HTL_extra_options -Wno-deprecated-declarations)

add_thunk()
project(SDL-1.2)

include("../ThunkAPI.cmake")

set(ALL_extra_includes
    "${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include"
    "${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include/SDL"
)
set(GTL_alias libSDL.so libSDL-1.2.so.0)
set(GTL_extra_links dl)

add_thunk()
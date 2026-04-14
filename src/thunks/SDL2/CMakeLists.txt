project(SDL2)

if(NOT LORE_SDL2_INCLUDE_DIR)
    message(WARNING "Thunk: LORE_SDL2_INCLUDE_DIR is not defined, skip SDL2 thunk configuration.")
endif()

include("../AddThunk.cmake")

set(ALL_extra_includes ${LORE_SDL2_INCLUDE_DIR})
set(GTL_alias libSDL2-2.0.so libSDL2-2.0.so.0)
set(GTL_extra_links dl)
set(GTL_extra_options -Wno-deprecated-declarations)
set(HTL_extra_options -Wno-deprecated-declarations)

add_thunk()
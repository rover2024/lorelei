project(X11)

include("../ThunkAPI.cmake")

set(GTL_alias libX11.so.6)
set(GTL_extra_options -Wno-deprecated-declarations)
set(HTL_extra_options -Wno-deprecated-declarations)

add_thunk()
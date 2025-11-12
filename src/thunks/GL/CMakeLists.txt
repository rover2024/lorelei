project(GL)

include("../ThunkAPI.cmake")

set(GTL_alias libGL.so.1)
set(GTL_force_links GLX_GTL)

add_thunk()
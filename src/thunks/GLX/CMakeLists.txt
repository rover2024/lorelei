project(GLX)

include("../AddThunk.cmake")

set(GTL_alias libGLX.so.0)
set(GTL_extra_links X11)
set(HTL_extra_links X11_HMW)

add_thunk()

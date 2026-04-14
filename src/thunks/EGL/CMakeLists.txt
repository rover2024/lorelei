project(EGL)

# Configure thunks
include("../AddThunk.cmake")

set(GTL_alias libEGL.so.1)

add_thunk()

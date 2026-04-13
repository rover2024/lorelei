project(zstd)

include("../AddThunk.cmake")

set(ALL_extra_includes ${LORE_ZSTD_INCLUDE_DIR})
set(GTL_alias libzstd.so.1)

add_thunk()

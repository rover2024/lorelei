project(vulkan)

include("../ThunkAPI.cmake")

set(ALL_extra_includes ${LORELEI_HOST_LIBRARIES_DIR}/Vulkan-Headers/include)
set(GTL_alias libvulkan.so.1)
set(HTL_extra_links X11_HMW xcb_HMW)

add_thunk()
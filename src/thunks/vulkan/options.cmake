if(LORELEI_ENABLE_GUEST)
    target_sources(${LORELEI_GUEST_PROJECT} PRIVATE vulkan/VulkanThunks.cpp)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/Vulkan-Headers/include"
    )
    set(LORELIB_GUEST_SYMLINKS libvulkan.so.1)
endif()

if(LORELEI_ENABLE_HOST)
    target_compile_features(${LORELEI_HOST_PROJECT} PRIVATE cxx_std_17)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/Vulkan-Headers/include"
    )
    target_link_libraries(${LORELEI_HOST_PROJECT} PRIVATE xcb GLX_HTL)
endif()

set(LORELIB_TLC_OPTIONS "-I${LORELEI_HOST_LIBRARIES_DIR}/Vulkan-Headers/include")
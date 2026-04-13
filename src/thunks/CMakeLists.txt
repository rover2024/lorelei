add_subdirectory(zlib)

if(LORE_BUILD_VULKAN_THUNK)
    add_subdirectory(vulkan)
endif()
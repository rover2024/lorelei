add_subdirectory(zlib)
add_subdirectory(OpenGL)
add_subdirectory(GLX)
add_subdirectory(GL)

if(LORE_BUILD_VULKAN_THUNK)
    add_subdirectory(vulkan)
endif()

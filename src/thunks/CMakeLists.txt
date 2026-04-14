set(_thunks
    zlib zstd # Compress
    EGL OpenGL GLX GL vulkan # Graphics
    SDL2 # Video
)

foreach(_thunk IN LISTS _thunks)
    if(LORE_DISABLE_THUNKS AND(${_thunk} IN_LIST LORE_DISABLE_THUNKS))
        continue()
    endif()

    add_subdirectory(${_thunk})
endforeach()

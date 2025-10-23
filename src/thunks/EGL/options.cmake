if(LORELEI_ENABLE_GUEST)
    set(LORELIB_GUEST_SYMLINKS libEGL.so.1)
    target_link_libraries(${LORELEI_GUEST_PROJECT} PRIVATE GLX)
    target_link_options(${LORELEI_GUEST_PROJECT} PRIVATE -Wl,--no-as-needed)
endif()

if(LORELEI_ENABLE_HOST)
    # Empty
endif()

set(LORELIB_TLC_OPTIONS)
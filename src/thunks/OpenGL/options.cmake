if(LORELEI_ENABLE_GUEST)
    set(LORELIB_GUEST_SYMLINKS libOpenGL.so.0 libOpenGL.so.0.0.0)
endif()

if(LORELEI_ENABLE_HOST)
    # Empty
endif()

set(LORELIB_TLC_OPTIONS)
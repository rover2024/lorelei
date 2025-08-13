if(LORELEI_ENABLE_GUEST)
    set(LORELIB_GUEST_SYMLINKS libpixman-1.so libpixman-1.so.0 libpixman-1.so.0.38.4)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "/usr/include/pixman-1"
    )
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "/usr/include/pixman-1"
    )
endif()

set(LORELIB_TLC_OPTIONS -I/usr/include/pixman-1)
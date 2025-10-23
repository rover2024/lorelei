if(LORELEI_ENABLE_GUEST)
    set(LORELIB_GUEST_SYMLINKS libX11.so.6 libX11.so.6.3.0)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "/usr/include/X11"
    )
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "/usr/include/X11"
    )
endif()

set(LORELIB_TLC_OPTIONS -I/usr/include/X11)

# set(LORELIB_SKIP_INSTALL TRUE)
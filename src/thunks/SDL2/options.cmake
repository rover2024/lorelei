if(LORELEI_ENABLE_GUEST)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "/home/overworld/Documents/ccxxprojs/SDL/include"
    )
    set(LORELIB_GUEST_SYMLINKS libSDL2-2.0.so.0 libSDL2-2.0.so)
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "/home/overworld/Documents/ccxxprojs/SDL/include"
    )
endif()

set(LORELIB_TLC_OPTIONS "-I/home/overworld/Documents/ccxxprojs/SDL/include")
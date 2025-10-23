if(LORELEI_ENABLE_GUEST)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include/SDL"
    )
    set(LORELIB_GUEST_SYMLINKS libSDL-1.2.so.0 libSDL.so)
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include/SDL"
    )
endif()

set(LORELIB_TLC_OPTIONS
    "-I${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include"
    "-I${LORELEI_HOST_LIBRARIES_DIR}/SDL-1.2/build/install/include/SDL"
)
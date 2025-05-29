if(LORELEI_ENABLE_GUEST)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/FFmpeg"
    )
    set(LORELIB_GUEST_SYMLINKS libavformat.so.62 libavformat.so.62.0.102)
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/FFmpeg"
    )
endif()

set(LORELIB_TLC_OPTIONS "-I${LORELEI_HOST_LIBRARIES_DIR}/FFmpeg")
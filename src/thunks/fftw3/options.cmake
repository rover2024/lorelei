if(LORELEI_ENABLE_GUEST)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10"
        "${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10/build/RelWithDebInfo"
    )
    set(LORELIB_GUEST_SYMLINKS libfftw3.so.3 libfftw3.so.3.6.9)
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10"
        "${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10/build/RelWithDebInfo"
    )
endif()

set(LORELIB_TLC_OPTIONS
    "-I${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10/build/install/include"
    "-I${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10"
    "-I${LORELEI_HOST_LIBRARIES_DIR}/fftw-3.3.10/build/RelWithDebInfo"
)
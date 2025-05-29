if(LORELEI_ENABLE_GUEST)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/openssl/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/openssl/include"
    )
    set(LORELIB_GUEST_SYMLINKS libcrypto.so.3)
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/openssl/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/openssl/include"
    )
endif()

set(LORELIB_TLC_OPTIONS
    "-I${LORELEI_HOST_LIBRARIES_DIR}/openssl/build/install/include"
    "-I${LORELEI_HOST_LIBRARIES_DIR}/openssl/include"
)
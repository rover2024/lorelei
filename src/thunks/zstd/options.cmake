if(LORELEI_ENABLE_GUEST)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/zstd/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/zstd/lib/common"
    )
    set(LORELIB_GUEST_SYMLINKS libzstd.so.1 libzstd.so.1.5.8)
endif()

if(LORELEI_ENABLE_HOST)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/zstd/build/install/include"
        "${LORELEI_HOST_LIBRARIES_DIR}/zstd/lib/common"
    )
endif()

set(LORELIB_TLC_OPTIONS
    "-I${LORELEI_HOST_LIBRARIES_DIR}/zstd/build/install/include"
    "-I${LORELEI_HOST_LIBRARIES_DIR}/zstd/lib/common"
)
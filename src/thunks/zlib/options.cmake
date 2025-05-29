if(LORELEI_ENABLE_GUEST)
    set_target_properties(${LORELEI_GUEST_PROJECT} PROPERTIES OUTPUT_NAME z)
    target_include_directories(${LORELEI_GUEST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/zlib/build/install/include"
    )
    set(LORELIB_GUEST_SYMLINKS libz.so.1)
endif()

if(LORELEI_ENABLE_HOST)
    set_target_properties(${LORELEI_HOST_PROJECT} PROPERTIES OUTPUT_NAME z_HTL)
    target_include_directories(${LORELEI_HOST_PROJECT} PRIVATE
        "${LORELEI_HOST_LIBRARIES_DIR}/zlib/build/install/include"
    )
endif()

set(LORELIB_TLC_OPTIONS "-I${LORELEI_HOST_LIBRARIES_DIR}/zlib/build/install/include")
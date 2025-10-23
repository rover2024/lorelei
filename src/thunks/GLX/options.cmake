if(LORELEI_ENABLE_GUEST)
    set(LORELIB_GUEST_SYMLINKS libGLX.so.0 libGLX.so.0.0.0)
    target_link_libraries(${LORELEI_GUEST_PROJECT} PRIVATE X11)
endif()

if(LORELEI_ENABLE_HOST)
    target_sources(${LORELEI_HOST_PROJECT} PRIVATE GLX/x11_mappings.cpp)
    target_compile_features(${LORELEI_HOST_PROJECT} PRIVATE cxx_std_17)
    target_link_libraries(${LORELEI_HOST_PROJECT} PRIVATE X11)
endif()

set(LORELIB_TLC_OPTIONS)

set(LORELIB_SKIP_INSTALL)
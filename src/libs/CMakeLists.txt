add_subdirectory(Core)

add_subdirectory(Utils)

if(LORELEI_ENABLE_GUEST)
    add_subdirectory(GuestRT)
endif()

if(LORELEI_ENABLE_HOST)
    add_subdirectory(HostRT)
endif()

add_subdirectory(Midware)

if(LORELEI_BUILD_TOOLS)
    add_subdirectory(TLC)
endif()
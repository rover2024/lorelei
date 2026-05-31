if(LORE_BUILD_GUEST_TARGETS)
    add_subdirectory(GuestRT)
endif()

add_subdirectory(HostRT)

if(NOT WIN32)
    add_subdirectory(EmulatorIntegration)
endif()

if(LORE_BUILD_MIDWARE)
    add_subdirectory(Midware)
endif()
if(LORE_BUILD_GUEST_TARGETS)
    add_subdirectory(GuestRuntime)
endif()

add_subdirectory(HostRuntime)

if(NOT WIN32)
    add_subdirectory(EmulatorIntegration)
endif()

add_subdirectory(Midwares)
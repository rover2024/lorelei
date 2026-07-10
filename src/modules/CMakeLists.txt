if(LORE_BUILD_GUEST_TARGETS)
    add_subdirectory(GuestRT)
endif()

# The host runtime runs host-native, loaded by the QEMU process; the guest never loads it. A guest
# cross build sets this OFF so its x86_64 tree does not carry a pointless cross-built LoreHostRT.
if(LORE_BUILD_HOST_TARGETS)
    add_subdirectory(HostRT)
endif()

# TODO: not used in this version
# add_subdirectory(QEMUThreadHook)
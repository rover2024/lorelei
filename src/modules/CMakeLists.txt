if(LORE_BUILD_GUEST_TARGETS)
    add_subdirectory(GuestRT)
endif()

add_subdirectory(HostRT)

# TODO: not used in this version
# add_subdirectory(QEMUThreadHook)
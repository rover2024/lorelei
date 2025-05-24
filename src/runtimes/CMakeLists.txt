add_subdirectory(shared)

if(LORELEI_ENABLE_GUEST)
    add_subdirectory(guest)
endif()

if(LORELEI_ENABLE_HOST)
    add_subdirectory(host)
endif()
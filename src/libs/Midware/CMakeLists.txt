if(LORELEI_ENABLE_GUEST)
    add_subdirectory(Guest)
endif()

if(LORELEI_ENABLE_HOST)
    add_subdirectory(Host)
endif()
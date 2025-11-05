project(_sample)

set(_GTL ${PROJECT_NAME}_GTL)
set(_HTL ${PROJECT_NAME}_HTL)

if(LORELEI_ENABLE_GUEST)
    add_library(${_GTL} SHARED Thunk_manifest_guest.cpp)

    set(_GTL_alias lib_sample.so.0)
    set_target_properties(${_GTL} PROPERTIES POST_LINK_ALIAS "${_GTL_alias}")

    lore_configure_target_common(${_GTL})
    lore_configure_guest_thunk(${_GTL})
    lore_guest_thunk_disable_register(${_GTL})
    target_link_libraries(${_GTL} PUBLIC LoreGuestRT)
endif()

if(LORELEI_ENABLE_HOST)
    add_library(${_HTL} SHARED Thunk_manifest_host.cpp)

    lore_configure_target_common(${_HTL})
    lore_configure_host_thunk(${_HTL})
    lore_host_thunk_disable_register(${_HTL})
    target_link_libraries(${_HTL} PUBLIC LoreHostRT)
endif()

if(LORELEI_BUILD_TOOLS)
    add_subdirectory(plugin)
endif()
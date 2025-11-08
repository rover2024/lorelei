project(tst_auto)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG QUIET COMPONENTS unit_test_framework)

if(NOT Boost_FOUND)
    message(WARNING "Boost not found, auto tests will not be built")
    return()
endif()

function(lore_add_auto_test _src)
    get_filename_component(_name ${_src} NAME_WE)
    add_executable(${_name} ${_src})
    target_link_libraries(${_name} PRIVATE Boost::unit_test_framework)
    add_test(NAME ${_name} COMMAND $<TARGET_FILE:${_name}>)
endfunction()

add_subdirectory(Core)

add_subdirectory(Utils)
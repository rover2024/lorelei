set(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG QUIET COMPONENTS unit_test_framework)

if(NOT Boost_FOUND)
    message(WARNING "Boost not found, auto tests will not be built")
    return()
endif()

# add_auto_test(<source> [extra link libraries...])
function(add_auto_test _src)
    get_filename_component(_name ${_src} NAME_WE)
    add_executable(tst_${_name} ${_src})
    target_include_directories(tst_${_name} PRIVATE
        ${LORE_SOURCE_DIR}/include
        ${LORE_BUILD_INCLUDE_DIR}
    )
    target_link_libraries(tst_${_name} PRIVATE Boost::unit_test_framework ${ARGN})
    add_test(NAME tst_${_name} COMMAND $<TARGET_FILE:tst_${_name}>)
endfunction()

add_subdirectory(DLCall)

add_subdirectory(Support)

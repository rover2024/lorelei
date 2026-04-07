set(Boost_USE_STATIC_LIBS ON)
find_package(Boost CONFIG QUIET COMPONENTS unit_test_framework)

if(NOT Boost_FOUND)
    message(WARNING "Boost not found, auto tests will not be built")
    return()
endif()

add_subdirectory(PassThrough)
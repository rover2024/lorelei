# The TLC test checks the source TLC generates for the ThunkExample fixture. CMake runs the
# generation here (and compiles it, so a malformed emission fails the build); the test then just
# inspects the generated source.
if(NOT TARGET LoreTLC)
    return()
endif()

set(_fixture ${CMAKE_CURRENT_SOURCE_DIR}/TestData)
set(_incs -I${LORE_SOURCE_DIR}/include -I${LORE_BUILD_INCLUDE_DIR} -I${_fixture})
set(_stat ${CMAKE_CURRENT_BINARY_DIR}/ThunkStat.json)
set(_host_src ${CMAKE_CURRENT_BINARY_DIR}/Thunk_host.cpp)
set(_guest_src ${CMAKE_CURRENT_BINARY_DIR}/Thunk_guest.cpp)

add_custom_command(OUTPUT ${_stat}
    COMMAND $<TARGET_FILE:LoreTLC> stat -o ${_stat}
        -c ${_fixture}/Symbols.conf ${_fixture}/Desc.h -- -xc++ ${_incs}
    DEPENDS LoreTLC ${_fixture}/Symbols.conf ${_fixture}/Desc.h ${_fixture}/ThunkExample.h
    VERBATIM
)
add_custom_command(OUTPUT ${_host_src}
    COMMAND $<TARGET_FILE:LoreTLC> generate -o ${_host_src} -s ${_stat} -m host
        ${_fixture}/Manifest_host.cpp -- -xc++ ${_incs}
    DEPENDS LoreTLC ${_stat} ${_fixture}/Manifest_host.cpp ${_fixture}/Desc.h
        ${_fixture}/LongDoubleConvert.h
    VERBATIM
)
add_custom_command(OUTPUT ${_guest_src}
    COMMAND $<TARGET_FILE:LoreTLC> generate -o ${_guest_src} -s ${_stat} -m guest
        ${_fixture}/Manifest_guest.cpp -- -xc++ -target x86_64-pc-linux-gnu ${_incs}
    DEPENDS LoreTLC ${_stat} ${_fixture}/Manifest_guest.cpp ${_fixture}/Desc.h
    VERBATIM
)

# Compile the generated sources so an invalid emission fails the build (the "does it compile" check).
add_library(tst_TLC_generated OBJECT ${_host_src} ${_guest_src})
target_compile_features(tst_TLC_generated PRIVATE cxx_std_20)
target_include_directories(tst_TLC_generated PRIVATE
    ${LORE_SOURCE_DIR}/include ${LORE_BUILD_INCLUDE_DIR} ${_fixture})

add_auto_test(tst_TLC.cpp)
add_dependencies(tst_TLC tst_TLC_generated)
target_compile_definitions(tst_TLC PRIVATE
    LORE_TLC_STAT="${_stat}"
    LORE_TLC_HOST_SRC="${_host_src}"
    LORE_TLC_GUEST_SRC="${_guest_src}"
)

# Export the fixture and the generated sources so the manual end-to-end test (src/tests/manual/TLC)
# can build real GTL/HTL libraries from them instead of regenerating.
set(LORE_TLC_FIXTURE_DIR ${_fixture} CACHE INTERNAL "ThunkExample fixture directory")
set(LORE_TLC_GEN_GUEST_SRC ${_guest_src} CACHE INTERNAL "Generated guest thunk source")
set(LORE_TLC_GEN_HOST_SRC ${_host_src} CACHE INTERNAL "Generated host thunk source")

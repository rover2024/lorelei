project(LoreQEMUPlugin)

# The plugin needs deferred binding of QEMU plugin APIs
string(REPLACE "-Wl,-z,defs" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    LINKS LoreHostRT
)
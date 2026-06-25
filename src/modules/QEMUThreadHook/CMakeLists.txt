project(LoreQEMUThreadHook)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    LINKS LoreHostRT
)
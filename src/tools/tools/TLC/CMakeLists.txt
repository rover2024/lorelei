project(LoreTLC)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_executable(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS_PRIVATE LoreToolMain LoreTLCApi
    DEFINES TOOL_VERSION="${LORE_VERSION}"
    INCLUDE_PRIVATE .
)
lore_link_clang(${PROJECT_NAME} PRIVATE)

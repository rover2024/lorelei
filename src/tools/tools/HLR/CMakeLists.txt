project(LoreHLR)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_executable(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS LoreHLRUtils LoreToolBasic
    DEFINES TOOL_VERSION="${LORE_VERSION}"
)
lore_link_clang(${PROJECT_NAME} PRIVATE)
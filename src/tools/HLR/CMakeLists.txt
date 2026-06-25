project(LoreHLR)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_executable(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS_PRIVATE LoreToolUtils LoreTLCApi LoreToolMain
    DEFINES TOOL_VERSION="${LORE_VERSION}"
    INCLUDE_PRIVATE .
)
lore_link_clang(${PROJECT_NAME} PRIVATE)

lore_add_resource(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/Resources/FileContext.h.in")
lore_add_resource(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Warning.txt.in")

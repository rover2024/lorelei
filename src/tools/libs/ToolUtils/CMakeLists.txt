project(LoreToolUtils)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME} STATIC
    SOURCES ${_src}
    LINKS LoreSupport
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/lorelei/Tools/ToolUtils
)
lore_link_clang(${PROJECT_NAME} PUBLIC)
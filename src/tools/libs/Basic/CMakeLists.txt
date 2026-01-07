project(LoreToolBasic)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS LoreCore
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/LoreTools/Basic
)
lore_link_clang(${PROJECT_NAME} PUBLIC)
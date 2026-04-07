project(LoreSupport)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS_PRIVATE $<BUILD_INTERFACE:llvm-standalone>
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/lorelei/Base/Support
)
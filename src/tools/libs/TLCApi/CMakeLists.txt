project(LoreTLCApi)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    LINKS LoreToolUtils
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/lorelei/Tools/TLCApi
)
project(LoreRTBase)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS LoreCore
    LINKS_PRIVATE $<BUILD_INTERFACE:json11>
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/LoreBase/RuntimeBase
)
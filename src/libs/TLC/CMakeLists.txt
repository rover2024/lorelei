project(LoreLibTLC)

file(GLOB_RECURSE _src *.h *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORELEI_SOURCE_DIR}/include/lorelei/TLC
)
lore_configure_target_common(${PROJECT_NAME})
lore_link_clang(${PROJECT_NAME} PUBLIC)
lore_configure_library(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE stdcorelib)
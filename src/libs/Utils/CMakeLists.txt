project(LoreUtils)

file(GLOB_RECURSE _src *.h *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORELEI_SOURCE_DIR}/include/lorelei/Utils
    ${LORELEI_SOURCE_DIR}/include/lorelei-c/Utils
)
lore_configure_target_common(${PROJECT_NAME})
lore_configure_library(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC stdcorelib)
project(LoreCore)

file(GLOB_RECURSE _src *.h *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORELEI_SOURCE_DIR}/include/lorelei/Core
    ${LORELEI_SOURCE_DIR}/include/lorelei-c/Core
)
lore_configure_target_common(${PROJECT_NAME})
lore_configure_library(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE json11 mpaland_printf)
target_link_libraries(${PROJECT_NAME} PUBLIC stdcorelib)

# lore_add_pkgconfig_dependency(${PROJECT_NAME} libffi REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE avcall)
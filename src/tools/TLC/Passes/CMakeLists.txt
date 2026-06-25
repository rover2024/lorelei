project(LoreTLC)

file(GLOB_RECURSE _src *.h *.c *.cpp)
add_executable(${PROJECT_NAME} ${_src})

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORELEI_SOURCE_DIR}/include/lorelei/TLC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
lore_configure_target_common(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE LoreLibTLC LoreUtils stdcorelib)
lore_configure_tool(${PROJECT_NAME})
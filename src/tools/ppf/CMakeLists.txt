project(LorePPF)

file(GLOB_RECURSE _src *.h *.c *.cpp)
add_executable(${PROJECT_NAME} ${_src})

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
lore_configure_target_common(${PROJECT_NAME})
lore_configure_tool(${PROJECT_NAME})
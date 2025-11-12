project(_sample_TLCPlugin)

file(GLOB_RECURSE _src *.h *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

lore_configure_target_common(${PROJECT_NAME})
lore_configure_plugin(${PROJECT_NAME} LoreTLC)

target_link_libraries(${PROJECT_NAME} PRIVATE LoreLibTLC)
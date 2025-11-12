project(X11_HMW)

file(GLOB_RECURSE _src *.h *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})
lore_configure_target_common(${PROJECT_NAME})
lore_configure_host_thunk(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE)

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORELEI_SOURCE_DIR}/include/lorelei/Midware/Host/X11
)

target_link_libraries(${PROJECT_NAME} PRIVATE X11)

lore_generate_global_header(${PROJECT_NAME} lorelei/Midware/Host/X11)

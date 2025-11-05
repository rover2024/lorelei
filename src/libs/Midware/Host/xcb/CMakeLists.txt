project(xcb_HMW)

file(GLOB_RECURSE _src *.h *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})
lore_configure_target_common(${PROJECT_NAME})
lore_configure_host_thunk(${PROJECT_NAME})

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORELEI_SOURCE_DIR}/include/lorelei/Midware/Host/xcb
)

lore_add_pkgconfig_dependency(${PROJECT_NAME} xcb)

lore_generate_global_header(${PROJECT_NAME} lorelei/Midware/Host/xcb)
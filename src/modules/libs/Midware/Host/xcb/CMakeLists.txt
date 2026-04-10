project(xcb_HMW)

find_library(_xcb_lib xcb)

if(NOT _xcb_lib)
    message(WARNING "xcb library not found")
    return()
endif()

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_host_thunk(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    LINKS_PRIVATE LoreSupport xcb
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_SOURCE_DIR}/include/lorelei/Modules/Midware/Host/xcb
)
lore_generate_global_header(${PROJECT_NAME} lorelei/Modules/Midware/Host/xcb)

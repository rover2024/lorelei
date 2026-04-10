project(X11_HMW)

find_library(_x11_lib X11)

if(NOT _x11_lib)
    message(WARNING "X11 library not found")
    return()
endif()

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_host_thunk(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    LINKS_PRIVATE LoreSupport ${_x11_lib}
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_SOURCE_DIR}/include/lorelei/Modules/Midware/Host/X11
)
lore_generate_global_header(${PROJECT_NAME} lorelei/Modules/Midware/Host/X11)

project(LorePassThrough)

file(GLOB_RECURSE _src *.h *.cpp)

# Use a tiny JSON library instead of linking against the huge LLVM JSON library
lore_add_library(${PROJECT_NAME} SHARED
    SOURCES ${_src}
    LINKS LoreSupport
    LINKS_PRIVATE $<BUILD_INTERFACE:json11> $<BUILD_INTERFACE:printf> $<BUILD_INTERFACE:llvm-standalone>
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/lorelei/Base/PassThrough
)

find_library(_avcall_lib avcall)

if(NOT _avcall_lib)
    message(FATAL_ERROR "avcall library not found")
endif()

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${_avcall_lib})
else()
    if(FFCALL_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${FFCALL_INCLUDE_DIR})
    endif()

    if(FFCALL_LIBRARY_DIR)
        target_link_directories(${PROJECT_NAME} PRIVATE ${FFCALL_LIBRARY_DIR})
        target_link_libraries(${PROJECT_NAME} PRIVATE avcall.lib)
    endif()
endif()

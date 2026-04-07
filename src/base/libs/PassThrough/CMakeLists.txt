project(LorePassThrough)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS LoreSupport
    LINKS_PRIVATE $<BUILD_INTERFACE:json11> $<BUILD_INTERFACE:printf>
)
qm_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORE_INCLUDE_DIR}/lorelei/Base/PassThrough
)

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE avcall)
else()
    if(FFCALL_INCLUDE_DIR)
        target_include_directories(${PROJECT_NAME} PRIVATE ${FFCALL_INCLUDE_DIR})
    endif()

    if(FFCALL_LIBRARY_DIR)
        target_link_directories(${PROJECT_NAME} PRIVATE ${FFCALL_LIBRARY_DIR})
        target_link_libraries(${PROJECT_NAME} PRIVATE avcall.lib)
    endif()
endif()

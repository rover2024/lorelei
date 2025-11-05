project(LoreHostRT)

file(GLOB_RECURSE _src *.h *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

lore_include_recursive(${PROJECT_NAME} PRIVATE
    ${LORELEI_SOURCE_DIR}/include/lorelei/HostRT
    ${LORELEI_SOURCE_DIR}/include/lorelei-c/HostRT
)
lore_configure_target_common(${PROJECT_NAME})
lore_configure_library(${PROJECT_NAME})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads dl)
target_link_libraries(${PROJECT_NAME} PUBLIC LoreCore)
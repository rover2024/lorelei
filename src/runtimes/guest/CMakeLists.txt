project(loregrt LANGUAGES C)

file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../../include>
)
target_compile_definitions(${PROJECT_NAME} PUBLIC LORELEI_GUEST_LIBRARY)

target_include_directories(${PROJECT_NAME} PRIVATE ../../../include/lorelei)
target_link_libraries(${PROJECT_NAME} PRIVATE loreshared dl)
target_link_options(${PROJECT_NAME} PRIVATE -pthread)
target_compile_definitions(${PROJECT_NAME} PRIVATE LORELEI_LIBRARY)

lorelei_install_library(${PROJECT_NAME})
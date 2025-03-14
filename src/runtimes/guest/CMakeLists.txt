project(loregrt LANGUAGES C)

file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

add_subdirectory(../shared shared)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../../include>
)

target_include_directories(${PROJECT_NAME} PRIVATE ../../../include/lorelei)

target_compile_definitions(${PROJECT_NAME} PUBLIC LORELEI_GUEST_LIBRARY)

lorelei_install_library(${PROJECT_NAME})
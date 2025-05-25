project(lorehrt LANGUAGES C CXX)

file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
add_library(${PROJECT_NAME} SHARED ${_src})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../../include>
)
target_compile_definitions(${PROJECT_NAME} PUBLIC LORELEI_HOST_LIBRARY)

target_include_directories(${PROJECT_NAME} PRIVATE ../../../include/lorelei)
target_link_libraries(${PROJECT_NAME} PRIVATE loreshared json11 dl)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE -fvisibility=hidden)
target_compile_definitions(${PROJECT_NAME} PRIVATE LORELEI_LIBRARY)

lorelei_install_library(${PROJECT_NAME})
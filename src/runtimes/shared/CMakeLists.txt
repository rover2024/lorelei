project(loreshared LANGUAGES C)

file(GLOB_RECURSE _src *.h *.hpp *.c *.cpp)
add_library(${PROJECT_NAME} STATIC ${_src})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../../include>
)
target_include_directories(${PROJECT_NAME} PRIVATE ../../../include/lorelei)
target_link_libraries(${PROJECT_NAME} PRIVATE ff_scanf mpaland_printf avcall dl)
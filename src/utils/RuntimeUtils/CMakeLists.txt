project(RuntimeUtils
    LANGUAGES CXX
)

file(GLOB_RECURSE _src *.h *.cpp)
add_library(${PROJECT_NAME} STATIC ${_src})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

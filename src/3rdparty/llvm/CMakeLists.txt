project(llvm-standalone)

file(GLOB_RECURSE _src *.h *.cpp)
lore_add_library(${PROJECT_NAME} STATIC NO_INSTALL
    SOURCES ${_src}
)
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
)
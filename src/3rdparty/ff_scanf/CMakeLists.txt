project(ff_scanf)

lorelei_add_simple_library(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE m)
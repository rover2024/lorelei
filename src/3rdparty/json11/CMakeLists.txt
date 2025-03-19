project(json11)

lorelei_add_simple_library(${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
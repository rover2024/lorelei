project(json11)

add_library(${PROJECT_NAME} STATIC ${LORELEI_SOURCE_DIR}/external/json11/json11.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${LORELEI_SOURCE_DIR}/external/json11>)
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC -fno-rtti -fno-exceptions -w)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
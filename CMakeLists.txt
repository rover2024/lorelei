cmake_minimum_required(VERSION 3.17)

project(lorelei VERSION 0.0.0.1 LANGUAGES C CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(LORELEI_ENABLE_GUEST "Build guest targets" OFF)
option(LORELEI_ENABLE_HOST "Build host targets" ON)
option(LORELEI_BUILD_TOOLS "Build tool targets" ON)
option(LORELEI_BUILD_THUNKS "Build thunk targets" ON)
option(LORELEI_BUILD_TESTS "Build test targets" OFF)
option(LORELEI_BUILD_EXAMPLES "Build example targets" OFF)
option(LORELEI_INSTALL "Install library" ON)
option(LORELEI_VCPKG_TOOLS_HINT "Install executables to tools directory" OFF)

# ----------------------------------
# CMake Settings
# ----------------------------------
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")

if(LORELEI_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(LORELEI_VERSION ${PROJECT_VERSION})
set(LORELEI_INSTALL_NAME ${PROJECT_NAME})
set(LORELEI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ----------------------------------
# Add source modules
# ----------------------------------
include("external/Findqmsetup.cmake")

# ----------------------------------
# Main
# ----------------------------------
set(LORELEI_GUEST_ARCH "x86_64")
set(LORELEI_GUEST_FIXED_REGISTER "r11")

if(NOT DEFINED LORELEI_HOST_ARCH)
    set(_arch ${CMAKE_SYSTEM_PROCESSOR})

    if(_arch MATCHES "x86_64|amd64")
        set(LORELEI_HOST_ARCH "x86_64")
        set(LORELEI_HOST_FIXED_REGISTER "r11")
    elseif(_arch MATCHES "aarch64|arm64")
        set(LORELEI_HOST_ARCH "aarch64")
        set(LORELEI_HOST_FIXED_REGISTER "x16")
    elseif(_arch MATCHES "riscv64")
        set(LORELEI_HOST_ARCH "riscv64")
        set(LORELEI_HOST_FIXED_REGISTER "t1")
    endif()
endif()

if(NOT DEFINED LORELEI_THUNK_SOURCES_DIR)
    set(LORELEI_THUNK_GENERATE on)
    set(LORELEI_THUNK_SOURCES_DIR ${QMSETUP_BUILD_DIR}/src/thunks)
else()
    set(LORELEI_THUNK_GENERATE off)
endif()

add_subdirectory(external)

add_subdirectory(src)

if(LORELEI_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(LORELEI_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

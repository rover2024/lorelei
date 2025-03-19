cmake_minimum_required(VERSION 3.17)

project(lorelei VERSION 0.0.0.1 LANGUAGES C CXX)

# ----------------------------------
# Build Options
# ----------------------------------
option(LORELEI_ENABLE_GUEST "Build guest targets" OFF)
option(LORELEI_BUILD_TOOLS "Build tool targets" ON)
option(LORELEI_BUILD_TESTS "Build test targets" ON)
option(LORELEI_INSTALL "Install library" ON)
option(LORELEI_VCPKG_TOOLS_HINT "Install executables to tools directory" OFF)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()
elseif(MINGW)
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif()

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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

add_subdirectory(src)

if (LORELEI_BUILD_TESTS)
    add_subdirectory(tests)
endif()
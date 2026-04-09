cmake_minimum_required(VERSION 3.19)

project(lorelei
    VERSION 0.0.1.0
    LANGUAGES C CXX
    DESCRIPTION "Cross-Architecture Compatibility Layer For Binary Translators."
    HOMEPAGE_URL "https://github.com/rover2024/lorelei"
)

# ----------------------------------
# Build Options
# ----------------------------------
# option(LORE_BUILD_SHARED "Build shared libraries" ON)
# option(LORE_BUILD_STATIC "Build static libraries" OFF)
option(LORE_BUILD_GUEST_TARGETS "Build guest targets" OFF)
option(LORE_BUILD_TOOLS "Build tool targets" ON)
option(LORE_BUILD_THUNKS "Build thunk targets" ON)
option(LORE_BUILD_TESTS "Build test targets" OFF)
option(LORE_DEVEL "Install developer files" ON)
option(LORE_INSTALL "Install library" ON)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    # if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
    #     set(CMAKE_DEBUG_POSTFIX "d")
    # endif()
elseif(MINGW)
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_IMPORT_LIBRARY_PREFIX "")
endif()

if(LORE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(LORE_VERSION ${PROJECT_VERSION})
set(LORE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(LORE_HOMEPAGE_URL ${PROJECT_HOMEPAGE_URL})
set(LORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ----------------------------------
# Add external modules
# ----------------------------------
add_subdirectory(external)

# ----------------------------------
# Initialize QMSetup (Required)
# ----------------------------------
find_package(qmsetup REQUIRED)

qm_import(Filesystem)

if(NOT DEFINED QMSETUP_BUILD_DIR)
    set(QMSETUP_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/out")
endif()

qm_init_directories()

# ----------------------------------
# Add source modules
# ----------------------------------
add_subdirectory(src)
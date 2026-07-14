cmake_minimum_required(VERSION 3.19)

project(lorelei
    VERSION 1.0.8.0
    LANGUAGES C CXX
    DESCRIPTION "Cross-Architecture Compatibility Layer For Binary Translators."
    HOMEPAGE_URL "https://github.com/rover2024/lorelei"
)

# ----------------------------------
# Build Options
# ----------------------------------
option(LORE_BUILD_GUEST_TARGETS "Build guest targets" OFF)
option(LORE_BUILD_HOST_TARGETS "Build host targets" ON)
option(LORE_BUILD_TOOLS "Build tool targets" ON)
option(LORE_BUILD_TESTS "Build test targets" OFF)
option(LORE_DEVEL "Install developer files" ON)
option(LORE_INSTALL "Install library" ON)

# Statically link LLVM/Clang into the tools (LoreTLC) and bundle Clang's builtin headers, so the
# installed toolchain is self-contained and can be copied to a machine without LLVM/Clang. Off by
# default: a normal build links libLLVM dynamically.
option(LORE_STATIC_LLVM "Statically link LLVM/Clang into the tools" OFF)

# Emitted into <lorelei/BuildConfig.h>
set(LORE_CONFIG_THUNK_VARG_MAX 64) # max packed variadic arguments a thunked call marshals
set(LORE_CONFIG_QEMU_SUPPORT_ADDRESS_SEPARATION OFF) # ON assumes the fork splits guest/host ranges at emuAddr. OFF classifies host addresses via dladdr and runs on stock QEMU

# ----------------------------------
# CMake Settings
# ----------------------------------
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
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
# Initialize QMSetup (Required)
# ----------------------------------
find_package(qmsetup REQUIRED)

qm_import(Filesystem)

if(NOT DEFINED QMSETUP_BUILD_DIR)
    set(QMSETUP_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/out")
endif()

qm_init_directories()

# ----------------------------------
# Tests
# ----------------------------------
if(LORE_BUILD_TESTS)
    enable_testing()
endif()

# ----------------------------------
# Add source modules
# ----------------------------------
add_subdirectory(src)

# Install LoreMakeThunk.py, the one-command thunk generator, next to the toolchain so an unpacked devkit
# can turn a library into a thunk with just its own LoreTLC and clang. Tools are host-side only.
if(LORE_INSTALL AND LORE_BUILD_TOOLS)
    install(PROGRAMS scripts/LoreMakeThunk.py
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

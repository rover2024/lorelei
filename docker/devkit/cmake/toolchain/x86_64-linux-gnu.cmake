# x86_64 guest cross toolchain file for the lorelei devkit.
#
# Installed into the devkit prefix at share/lorelei/toolchains/x86_64-linux-gnu.cmake, and passed as
# CMAKE_TOOLCHAIN_FILE for the guest (GTL) build. It targets x86_64 with the toolchain's own bundled
# clang and the self-contained x86_64 sysroot, so a thunk author needs no system x86_64 dev packages
# and no separate cross compiler. MakeThunk.py wires this in automatically when given a devkit prefix.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# The devkit prefix is three levels up from this file (share/lorelei/toolchains/).
get_filename_component(_LORELEI_DEVKIT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

set(_lorelei_sysroot "${_LORELEI_DEVKIT}/x86_64")
set(CMAKE_SYSROOT "${_lorelei_sysroot}")

# One clang binary cross-targets x86_64 via --target (set through CMAKE_*_COMPILER_TARGET). Use it
# directly, not the x86_64-linux-gnu-clang wrapper, so the target/sysroot are visible to CMake's own
# compiler-id checks.
set(CMAKE_C_COMPILER "${_LORELEI_DEVKIT}/bin/clang")
set(CMAKE_CXX_COMPILER "${_LORELEI_DEVKIT}/bin/clang++")
set(CMAKE_C_COMPILER_TARGET x86_64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET x86_64-linux-gnu)
# The ASM target must be set too: assembly (e.g. TinyCoroutine's per-arch .S) is otherwise assembled for
# clang's default target, which happens to be x86_64 on an x86_64 build host but is wrong on an arm64
# host, feeding x86_64 asm to the aarch64 assembler.
set(CMAKE_ASM_COMPILER "${_LORELEI_DEVKIT}/bin/clang")
set(CMAKE_ASM_COMPILER_TARGET x86_64-linux-gnu)

# Link with the bundled lld, not a system x86_64 binutils that a user (or an arm64 host) may not have.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")

# Resolve libraries, headers and packages inside the guest sysroot; run build programs (cmake, ninja,
# the host TLC) from the host. qmsetup_DIR / lorelei_DIR are still passed as absolute hints, which take
# precedence over this rerooting.
set(CMAKE_FIND_ROOT_PATH "${_lorelei_sysroot}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

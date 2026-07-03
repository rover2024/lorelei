# CMake toolchain for cross-compiling to aarch64 on a Debian/Ubuntu multiarch amd64 host, with the
# gcc-aarch64-linux-gnu cross toolchain against the :arm64 multiarch libraries. Used by the deploy
# build to produce the aarch64 host tree on an x86_64 runner (see docker/scripts/build-deploy.sh).
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# The multiarch library triplet, so find_library resolves /usr/lib/aarch64-linux-gnu/... (e.g. the
# aarch64 libavcall.a that LoreDLCall links) rather than the host's amd64 copy.
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

# No separate sysroot: the target libraries and headers live in the host's multiarch tree, so search
# both. Programs still run on the build host.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

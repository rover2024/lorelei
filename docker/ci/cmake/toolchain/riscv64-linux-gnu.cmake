# CMake toolchain for cross-compiling to riscv64 on a Debian/Ubuntu multiarch amd64 host, with the
# gcc-riscv64-linux-gnu cross toolchain against the :riscv64 multiarch libraries. Used by the deploy
# build to produce the riscv64 host tree on an x86_64 runner (see docker/scripts/build-deploy.sh).
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

set(CMAKE_C_COMPILER riscv64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER riscv64-linux-gnu-g++)

# The multiarch library triplet, so find_library resolves /usr/lib/riscv64-linux-gnu/... (e.g. the
# riscv64 libavcall.a that LoreDLCall links) rather than the host's amd64 copy.
set(CMAKE_LIBRARY_ARCHITECTURE riscv64-linux-gnu)

# No separate sysroot: the target libraries and headers live in the host's multiarch tree, so search
# both. Programs still run on the build host.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

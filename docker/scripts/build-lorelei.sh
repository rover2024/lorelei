#!/usr/bin/env bash
#
# Build this repository (lorelei v2) from the copied-in source, twice, into a uniform layout:
#   install/lorelei         host side: LoreTLC + LoreHostRT (this machine's arch), and the tests
#   install/x86_64/lorelei  guest side: LoreGuestRT (x86_64)
#
# On an x86_64 host the host build also enables the guest targets, so the in-tree ThunkExample test
# (run_manual_tlc) can build the GTL and HTL in one tree; on a cross (aarch64/riscv64) host that test
# is x86_64-only and skipped, and the guest side is cross-compiled for x86_64.
set -euo pipefail
: "${LORELEI_SRC:?}"
: "${INSTALL_DIR:?}"

arch="$(uname -m)"
if [ "$arch" = "x86_64" ] || [ "$arch" = "amd64" ]; then
    GUEST_CC=gcc
    GUEST_CXX=g++
    HOST_BUILD_GUEST=TRUE
else
    GUEST_CC=x86_64-linux-gnu-gcc
    GUEST_CXX=x86_64-linux-gnu-g++
    HOST_BUILD_GUEST=FALSE
fi

cd "$LORELEI_SRC"

# Host side (native), with the tests. The build tree is kept so the test run can drive it.
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/lorelei" \
    -Dqmsetup_DIR="$INSTALL_DIR/qmsetup/lib/cmake/qmsetup" \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=$HOST_BUILD_GUEST \
    -DLORE_BUILD_TESTS=ON
cmake --build build --target all
cmake --build build --target install

# Guest side: the x86_64 runtime, installed under install/x86_64.
cmake -B build-guest -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$GUEST_CC" \
    -DCMAKE_CXX_COMPILER="$GUEST_CXX" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/x86_64/lorelei" \
    -Dqmsetup_DIR="$INSTALL_DIR/qmsetup/lib/cmake/qmsetup" \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE
cmake --build build-guest --target install

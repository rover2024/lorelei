#!/usr/bin/env bash
#
# Build this repository (lorelei v2) from the copied-in source: the host tools (LoreTLC), both
# runtimes, and the tests. Installs into INSTALL_DIR and keeps the build tree so the test run can
# drive the in-tree end-to-end target.
set -euo pipefail
: "${LORELEI_SRC:?}"
: "${INSTALL_DIR:?}"

cd "$LORELEI_SRC"
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/lorelei" \
    -Dqmsetup_DIR="$INSTALL_DIR/qmsetup/lib/cmake/qmsetup" \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE \
    -DLORE_BUILD_TESTS=ON
cmake --build build --target all
cmake --build build --target install

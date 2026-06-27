#!/usr/bin/env bash
#
# Build the qmsetup CMake helper package that lorelei and the thunks depend on.
set -euo pipefail
: "${REPOS_DIR:?}"
: "${INSTALL_DIR:?}"

cd "$REPOS_DIR"
if [ ! -d qmsetup/.git ]; then
    git clone --recursive https://github.com/stdware/qmsetup.git
fi
cd qmsetup
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/qmsetup"
cmake --build build --target install

#!/usr/bin/env bash
#
# Build the qmsetup CMake helper package that lorelei and the thunks depend on. Fetched as source
# tarballs (qmsetup plus its stdcorelib submodule) rather than cloned, which is much faster over a
# proxy.
set -euo pipefail
: "${REPOS_DIR:?}"
: "${INSTALL_DIR:?}"

cd "$REPOS_DIR"
if [ ! -d qmsetup ]; then
    curl -fsSL https://codeload.github.com/stdware/qmsetup/tar.gz/refs/heads/main | tar xz
    mv qmsetup-main qmsetup
    # stdcorelib is a git submodule (src/stdcorelib); fetch it as a tarball too. qmsetup builds it
    # static and does not install it, so nothing downstream has to find_package it.
    curl -fsSL https://codeload.github.com/stdware/stdcorelib/tar.gz/refs/heads/main | tar xz
    rm -rf qmsetup/src/stdcorelib
    mv stdcorelib-main qmsetup/src/stdcorelib
fi
cd qmsetup
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
cmake --build build --target install

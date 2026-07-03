#!/usr/bin/env bash
#
# Build the qmsetup CMake helper package that lorelei and the thunks depend on. Fetched as source
# tarballs (qmsetup plus its syscmdline submodule) rather than cloned, which is much faster over a
# proxy.
set -euo pipefail
: "${REPOS_DIR:?}"
: "${INSTALL_DIR:?}"

cd "$REPOS_DIR"
if [ ! -d qmsetup ]; then
    curl -fsSL https://codeload.github.com/stdware/qmsetup/tar.gz/refs/heads/main | tar xz
    mv qmsetup-main qmsetup
    # syscmdline is a git submodule (src/syscmdline); fetch it as a tarball too.
    curl -fsSL https://codeload.github.com/SineStriker/syscmdline/tar.gz/refs/heads/main | tar xz
    rm -rf qmsetup/src/syscmdline
    mv syscmdline-main qmsetup/src/syscmdline
fi
cd qmsetup
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR"
cmake --build build --target install

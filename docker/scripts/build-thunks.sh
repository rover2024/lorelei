#!/usr/bin/env bash
#
# Clone and build lorelei-thunks against the installed lorelei, into the same prefix. Only the zlib
# thunk is built (SDL2 is disabled) to keep the image small; zlib is what the end-to-end test runs.
# Both sides are x86_64 here, so a single build produces the guest (GTL) and host (HTL) thunk.
set -euo pipefail
: "${REPOS_DIR:?}"
: "${INSTALL_DIR:?}"

cd "$REPOS_DIR"
if [ ! -d lorelei-thunks/.git ]; then
    git clone https://github.com/rover2024/lorelei-thunks.git
fi
cd lorelei-thunks
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/lorethunks" \
    -Dqmsetup_DIR="$INSTALL_DIR/qmsetup/lib/cmake/qmsetup" \
    -Dlorelei_DIR="$INSTALL_DIR/lorelei/lib/cmake/lorelei" \
    -DTHUNK_BUILD_HOST_TARGETS=TRUE \
    -DTHUNK_BUILD_GUEST_TARGETS=TRUE \
    -DTHUNK_DISABLE_LIBRARIES=SDL2
cmake --build build --target install

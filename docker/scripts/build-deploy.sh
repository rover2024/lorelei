#!/usr/bin/env bash
#
# Build the deployable runtime trees for upload. Reuses the toolchain, the staging lorelei install and
# the pre-generated thunk sources already present in the image (see build-lorelei.sh / build-thunks.sh)
# and rebuilds everything fresh into DEPLOY_DIR with the tools left out (LORE_BUILD_TOOLS=OFF) and no
# tests. Each target is a self-contained runtime tree with lorelei and the thunks merged into one flat
# prefix:
#
#   <deploy>/<arch>-host    host runtime (this machine's arch) + host thunks (HTL)
#   <deploy>/x86_64-guest   x86_64 guest runtime + guest thunks (GTL)   [x86_64 host only]
#
# The guest is always x86_64, so it is only built natively on an x86_64 host; a cross host produces
# just its own <arch>-host tree. TLC is never run here: the thunk sources come pre-generated via
# THUNK_GEN_SOURCE_DIR (installed by the image's host thunks build), which is what keeps these trees
# tool-free.
set -euo pipefail
: "${INSTALL_DIR:?}"
: "${LORELEI_SRC:?}"
: "${REPOS_DIR:?}"

DEPLOY_DIR="${1:-${DEPLOY_DIR:-$REPOS_DIR/deploy}}"
GEN_SOURCE_DIR="$INSTALL_DIR/share/lorelei/thunks"
QMSETUP_DIR="$INSTALL_DIR/lib/cmake/qmsetup"
THUNKS_SRC="$REPOS_DIR/lorelei-thunks"

arch="$(uname -m)"
[ "$arch" = "amd64" ] && arch=x86_64

# build_target <name> <guest-targets TRUE|FALSE>
# Build a tool-free lorelei plus its thunks into $DEPLOY_DIR/<name>, from fresh build dirs. The thunks
# reuse the pre-generated sources, so no TLC runs and lorelei can be the tool-free deploy install.
build_target() {
    local name="$1"
    local guest="$2"
    local prefix="$DEPLOY_DIR/$name"
    local lore_build="$REPOS_DIR/deploy-build/$name/lorelei"
    local thunk_build="$REPOS_DIR/deploy-build/$name/thunks"

    local thunk_host thunk_guest
    if [ "$guest" = "TRUE" ]; then
        thunk_host=FALSE
        thunk_guest=TRUE
    else
        thunk_host=TRUE
        thunk_guest=FALSE
    fi

    echo "== deploy $name: lorelei (no tools, no tests) =="
    cmake -S "$LORELEI_SRC" -B "$lore_build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" \
        -Dqmsetup_DIR="$QMSETUP_DIR" \
        -DLORE_BUILD_TOOLS=FALSE \
        -DLORE_BUILD_TESTS=OFF \
        -DLORE_BUILD_GUEST_TARGETS=$guest
    cmake --build "$lore_build" --target install

    echo "== deploy $name: thunks (no tools, pre-generated sources) =="
    cmake -S "$THUNKS_SRC" -B "$thunk_build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" \
        -Dqmsetup_DIR="$QMSETUP_DIR" \
        -Dlorelei_DIR="$prefix/lib/cmake/lorelei" \
        -DTHUNK_GEN_SOURCE_DIR="$GEN_SOURCE_DIR" \
        -DTHUNK_BUILD_HOST_TARGETS=$thunk_host \
        -DTHUNK_BUILD_GUEST_TARGETS=$thunk_guest
    cmake --build "$thunk_build" --target install
}

build_target "$arch-host" FALSE

# The guest is x86_64. Build it only when this is an x86_64 host, where it is native; a cross host
# leaves the guest tree to the x86_64 runner.
if [ "$arch" = "x86_64" ]; then
    build_target "x86_64-guest" TRUE
fi

echo "Deploy trees ready under $DEPLOY_DIR:"
ls -1 "$DEPLOY_DIR"

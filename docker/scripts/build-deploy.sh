#!/usr/bin/env bash
#
# Build the deployable runtime trees for upload. Reuses the toolchain, the staging lorelei install and
# the pre-generated thunk sources already present in the image (see build-lorelei.sh / build-thunks.sh)
# and rebuilds everything fresh into DEPLOY_DIR with the tools left out (LORE_BUILD_TOOLS=OFF) and no
# tests. Each target is a self-contained runtime tree with lorelei and the thunks merged into one flat
# prefix:
#
#   <deploy>/x86_64-host    x86_64 host runtime + host thunks (HTL)
#   <deploy>/x86_64-guest   x86_64 guest runtime + guest thunks (GTL)
#   <deploy>/aarch64-host   aarch64 host runtime + host thunks (HTL), cross-compiled   [x86_64 only]
#
# On an x86_64 host all of the above are produced: the two x86_64 trees natively, and the aarch64 host
# tree cross-compiled (the cross toolchain is installed on demand, see below). A non-x86_64 host builds
# only its own native <arch>-host tree.
#
# TLC is never run here: the thunk sources come pre-generated via THUNK_GEN_SOURCE_DIR (installed by the
# image's host thunks build), which is what keeps these trees tool-free. The pre-generated host source
# is reused as-is for the aarch64 cross target too: for the stable thunks (zlib, lzma) generation does
# not depend on the target arch, so the same Thunk_host.cpp compiles correctly for aarch64. A library
# whose generation IS arch-specific (variadic / va_list) would instead need a real cross-generate; that
# is a later step, on top of the -target support already in the thunks CMake.
set -euo pipefail
: "${INSTALL_DIR:?}"
: "${LORELEI_SRC:?}"
: "${REPOS_DIR:?}"

DEPLOY_DIR="${1:-${DEPLOY_DIR:-$REPOS_DIR/deploy}}"
GEN_SOURCE_DIR="$INSTALL_DIR/share/lorelei/thunks"
QMSETUP_DIR="$INSTALL_DIR/lib/cmake/qmsetup"
THUNKS_SRC="$REPOS_DIR/lorelei-thunks"
TOOLCHAIN_DIR="$LORELEI_SRC/docker/cmake/toolchain"

arch="$(uname -m)"
[ "$arch" = "amd64" ] && arch=x86_64

# build_target <name> <guest-targets TRUE|FALSE> [toolchain-file]
# Build a tool-free lorelei plus its thunks into $DEPLOY_DIR/<name>, from fresh build dirs. With a
# toolchain file the target is cross-compiled. The thunks reuse the pre-generated sources, so no TLC
# runs and lorelei can be the tool-free deploy install.
build_target() {
    local name="$1"
    local guest="$2"
    local toolchain="${3:-}"
    local prefix="$DEPLOY_DIR/$name"
    local lore_build="$REPOS_DIR/deploy-build/$name/lorelei"
    local thunk_build="$REPOS_DIR/deploy-build/$name/thunks"

    local tc_arg=()
    [ -n "$toolchain" ] && tc_arg=(-DCMAKE_TOOLCHAIN_FILE="$toolchain")

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
        "${tc_arg[@]}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" \
        -Dqmsetup_DIR="$QMSETUP_DIR" \
        -DLORE_BUILD_TOOLS=FALSE \
        -DLORE_BUILD_TESTS=OFF \
        -DLORE_BUILD_GUEST_TARGETS=$guest
    cmake --build "$lore_build" --target install

    echo "== deploy $name: thunks (no tools, pre-generated sources) =="
    cmake -S "$THUNKS_SRC" -B "$thunk_build" -G Ninja \
        "${tc_arg[@]}" \
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

# The guest is x86_64, and the cross host trees only make sense on an x86_64 build machine.
if [ "$arch" = "x86_64" ]; then
    build_target "x86_64-guest" TRUE

    # aarch64 host, cross-compiled. Install the cross toolchain on demand (via sudo, configured by the
    # image's bootstrap) so the shared image stays untouched. DEPLOY_CROSS=0 skips it.
    if [ "${DEPLOY_CROSS:-1}" = "1" ]; then
        echo "== installing the aarch64 cross toolchain =="
        sudo bash "$LORELEI_SRC/docker/scripts/bootstrap-cross.sh"
        build_target "aarch64-host" FALSE "$TOOLCHAIN_DIR/aarch64-linux-gnu.cmake"
    fi
fi

echo "Deploy trees ready under $DEPLOY_DIR:"
ls -1 "$DEPLOY_DIR"

#!/usr/bin/env bash
#
# Build the deployable runtime trees for upload. Reuses the toolchain and the staging lorelei install
# already present in the image (see build-lorelei.sh / build-thunks.sh) and rebuilds everything fresh
# into DEPLOY_DIR with the tools left out (LORE_BUILD_TOOLS=OFF) and no tests. Each target is a
# self-contained runtime tree with lorelei and the thunks merged into one flat prefix:
#
#   <deploy>/x86_64-host    x86_64 host runtime + host thunks (HTL)
#   <deploy>/x86_64-guest   x86_64 guest runtime + guest thunks (GTL)
#   <deploy>/aarch64-host   aarch64 host runtime + host thunks (HTL), cross-compiled   [x86_64 only]
#   <deploy>/riscv64-host   riscv64 host runtime + host thunks (HTL), cross-compiled   [x86_64 only]
#
# On an x86_64 host all of the above are produced: the two x86_64 trees natively (reusing the image's
# pre-generated x86_64 thunk sources), and the aarch64 and riscv64 host trees by real cross builds. A
# non-x86_64 host builds only its own native <arch>-host tree.
#
# The native trees never run TLC: their thunk sources come pre-generated via THUNK_GEN_SOURCE_DIR
# (installed by the image's host thunks build). The cross tree DOES cross-generate its own host source
# (a two-phase build, see cross_host_target) rather than reuse the x86_64 one, because generation is
# arch-specific for anything touching va_list / long double; the reuse only happens to be correct for
# the plain-ABI stable thunks.
set -euo pipefail
: "${INSTALL_DIR:?}"
: "${LORELEI_SRC:?}"
: "${REPOS_DIR:?}"

DEPLOY_DIR="${1:-${DEPLOY_DIR:-$REPOS_DIR/deploy}}"
GEN_SOURCE_DIR="$INSTALL_DIR/share/lorelei/thunks"
QMSETUP_DIR="$INSTALL_DIR/lib/cmake/qmsetup"
STAGING_LORELEI_DIR="$INSTALL_DIR/lib/cmake/lorelei"   # the with-tools staging lorelei (carries TLC)
THUNKS_SRC="$REPOS_DIR/lorelei-thunks"
TOOLCHAIN_DIR="$LORELEI_SRC/docker/cmake/toolchain"

arch="$(uname -m)"
[ "$arch" = "amd64" ] && arch=x86_64

# build_target <name> <guest-targets TRUE|FALSE>
# Native target: build a tool-free lorelei plus its thunks into $DEPLOY_DIR/<name> from fresh build
# dirs, reusing the image's pre-generated sources (no TLC).
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

# cross_host_target <arch> <toolchain-file> <tlc-host-parse-args>
# Cross target: produce $DEPLOY_DIR/<arch>-host in three steps, none of which reuse the x86_64 sources:
#   1. build the tool-free cross lorelei (the aarch64 host runtime);
#   2. cross-generate the host thunk source with the native x86_64 TLC targeting <arch> (generate only,
#      against the staging lorelei that carries a runnable TLC; the parse args point clang at the cross
#      toolchain's headers);
#   3. cross-compile the HTL from that generated source, linking the cross lorelei runtime.
cross_host_target() {
    local carch="$1"
    local toolchain="$2"
    local parse_args="$3"
    local name="$carch-host"
    local prefix="$DEPLOY_DIR/$name"
    local lore_build="$REPOS_DIR/deploy-build/$name/lorelei"
    local gen_out="$REPOS_DIR/deploy-build/$name/gen"
    local gen_build="$REPOS_DIR/deploy-build/$name/gen-build"
    local thunk_build="$REPOS_DIR/deploy-build/$name/thunks"

    echo "== deploy $name: lorelei (cross, no tools, no tests) =="
    cmake -S "$LORELEI_SRC" -B "$lore_build" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" \
        -Dqmsetup_DIR="$QMSETUP_DIR" \
        -DLORE_BUILD_TOOLS=FALSE \
        -DLORE_BUILD_TESTS=OFF \
        -DLORE_BUILD_GUEST_TARGETS=FALSE
    cmake --build "$lore_build" --target install

    echo "== deploy $name: cross-generate host thunk source (native TLC, -target $carch) =="
    # Native compiler (the TLC is an x86_64 executable), THUNK_HOST_ARCH forces the -target, and
    # THUNK_HTL_EXTRA_ARGS points clang's aarch64 header parse at the cross toolchain. Generate only,
    # against the staging lorelei that carries a runnable TLC. The sources install under gen_out.
    cmake -S "$THUNKS_SRC" -B "$gen_build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$gen_out" \
        -Dqmsetup_DIR="$QMSETUP_DIR" \
        -Dlorelei_DIR="$STAGING_LORELEI_DIR" \
        -DTHUNK_HOST_ARCH="$carch" \
        -DTHUNK_HTL_EXTRA_ARGS="$parse_args" \
        -DTHUNK_BUILD_HOST_TARGETS=FALSE \
        -DTHUNK_BUILD_GUEST_TARGETS=FALSE
    cmake --build "$gen_build" --target install

    echo "== deploy $name: thunks HTL (cross-compile the generated source) =="
    cmake -S "$THUNKS_SRC" -B "$thunk_build" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="$toolchain" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$prefix" \
        -Dqmsetup_DIR="$QMSETUP_DIR" \
        -Dlorelei_DIR="$prefix/lib/cmake/lorelei" \
        -DTHUNK_GEN_SOURCE_DIR="$gen_out/share/lorelei/thunks" \
        -DTHUNK_BUILD_HOST_TARGETS=TRUE \
        -DTHUNK_BUILD_GUEST_TARGETS=FALSE
    cmake --build "$thunk_build" --target install
}

build_target "$arch-host" FALSE

# The guest is x86_64, and the cross host trees only make sense on an x86_64 build machine.
if [ "$arch" = "x86_64" ]; then
    build_target "x86_64-guest" TRUE

    # Cross host trees (aarch64, riscv64), real cross builds. The cross toolchain for each is installed
    # on demand (via sudo, configured by the image's bootstrap) so the shared image stays untouched.
    # DEPLOY_CROSS=0 skips them.
    if [ "${DEPLOY_CROSS:-1}" = "1" ]; then
        for carch in aarch64 riscv64; do
            echo "== installing the $carch cross toolchain =="
            sudo bash "$LORELEI_SRC/docker/scripts/bootstrap-cross.sh" "$carch"
            cross_host_target "$carch" \
                "$TOOLCHAIN_DIR/$carch-linux-gnu.cmake" \
                "--gcc-toolchain=/usr;-idirafter;/usr/include"
        done
    fi
fi

echo "Deploy trees ready under $DEPLOY_DIR:"
ls -1 "$DEPLOY_DIR"

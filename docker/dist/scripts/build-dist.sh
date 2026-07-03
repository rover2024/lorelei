#!/usr/bin/env bash
#
# Build the lorelei distribution for one target arch and cut two tarballs from a single tree: a small
# runtime (for running thunked guests) and a full devkit (runtime + devel + toolchain, for building
# thunks). Both use the Scheme A layout: the host-arch side at the prefix root and the x86_64 guest
# side nested under x86_64/.
#
#   <tree>/                 host runtime + devel + LoreTLC + host thunks (HTL) + bundled clang/LLVM
#   <tree>/x86_64/          x86_64 guest runtime + devel + guest thunks (GTL) + guest sysroot
#
# runtime-<arch>.tar.xz  = the runtime .so's + thunks (HTL/GTL) + ThunkDB.json, no tools/headers/LLVM.
# devkit-<arch>.tar.xz   = the whole tree.
#
# Everything is built on an x86_64 host: the native arch natively, and aarch64/riscv64 by cross
# compilation (the guest x86_64 side is always native here). No target binary runs at build time.
set -euo pipefail
: "${LORELEI_SRC:?}"    # lorelei v2 source tree
: "${THUNKS_SRC:?}"     # lorelei-thunks source tree
: "${REPOS_DIR:?}"      # scratch dir for build trees and external deps
: "${OUT_DIR:?}"        # where the tarballs are written

TARGET="${1:?usage: build-dist.sh <x86_64|aarch64|riscv64>}"
LLVM_VER="${LLVM_VER:-20}"
GCC_VER="${GCC_VER:-14}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN_DIR="$SCRIPT_DIR/../cmake/toolchain"
COMMON_SCRIPTS="$LORELEI_SRC/docker/common"
build_arch="$(uname -m)"
[ "$build_arch" = "amd64" ] && build_arch=x86_64

TREE="$REPOS_DIR/dist/$TARGET"
rm -rf "$TREE"
mkdir -p "$TREE" "$OUT_DIR"

# Cross vs native. The host (target-arch) side cross-compiles when TARGET != build_arch; the x86_64
# guest side is always native on this x86_64 builder.
host_toolchain=()
CROSS=0
LLVM_SRC="/usr/lib/llvm-${LLVM_VER}"   # what bundle-llvm.sh bundles (native LLVM by default)
if [ "$TARGET" != "$build_arch" ]; then
    CROSS=1
    # Install the cross gcc, extract the target-arch clang/LLVM into a prefix and the thunk libs into
    # the multiarch tree. The target LLVM cannot live at /usr/lib/llvm-N (the native x86_64 clang does),
    # so link it dynamically from the prefix.
    "$SCRIPT_DIR/prepare-cross.sh" "$TARGET"
    LLVM_SRC="/opt/xllvm/$TARGET/usr/lib/llvm-${LLVM_VER}"
    triplet="$TARGET-linux-gnu"
    # libLLVM.so pulls in libffi/libedit/libzstd/libxml2 (extracted into the prefix's multiarch dir by
    # prepare-cross). GNU ld resolves a shared lib's own NEEDED deps via -rpath-link, not -L, so point
    # -rpath-link there; the -L covers any direct link against the target libs in the same dir.
    xllvm_libdir="/opt/xllvm/$TARGET/usr/lib/$triplet"
    link_flags="-L$xllvm_libdir -Wl,-rpath-link,$xllvm_libdir"
    host_toolchain=(
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/$TARGET-linux-gnu.cmake"
        -DClang_DIR="$LLVM_SRC/lib/cmake/clang"
        -DLLVM_DIR="$LLVM_SRC/lib/cmake/llvm"
        -DLORE_STATIC_LLVM=OFF
        -DCMAKE_EXE_LINKER_FLAGS="$link_flags"
        -DCMAKE_SHARED_LINKER_FLAGS="$link_flags"
    )
fi

# --- 1. qmsetup (build tool, native x86_64) --------------------------------------------------------
# Used at build time by the lorelei/thunks builds. A native x86_64 qmsetup is enough for building the
# tree; the target-arch qmsetup the end user needs is added to the devkit by prepare_devkit_qmsetup.
INSTALL_DIR="$REPOS_DIR/qmsetup-native" bash "$COMMON_SCRIPTS/build-qmsetup.sh"
QMSETUP_NATIVE="$REPOS_DIR/qmsetup-native/lib/cmake/qmsetup"

# --- 2. lorelei host side (target arch): LoreTLC + LoreHostRT ---------------------------------------
cd "$LORELEI_SRC"
cmake -B "$REPOS_DIR/build/$TARGET-host" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$TREE" \
    -Dqmsetup_DIR="$QMSETUP_NATIVE" \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=FALSE \
    -DLORE_BUILD_TESTS=OFF \
    "${host_toolchain[@]}"
cmake --build "$REPOS_DIR/build/$TARGET-host" --target install

# --- 2b. native staging lorelei (x86_64, runnable LoreTLC) for cross thunk generation --------------
# On a cross target the LoreTLC in $TREE is target-arch and cannot run here, so thunk sources are
# generated with this native TLC (targeting $TARGET, like the deploy cross build). Built once, reused.
STAGING="$REPOS_DIR/staging"
if [ "$CROSS" = "1" ] && [ ! -x "$STAGING/bin/LoreTLC" ]; then
    cmake -B "$REPOS_DIR/build/staging" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$STAGING" \
        -Dqmsetup_DIR="$QMSETUP_NATIVE" \
        -DLORE_BUILD_TOOLS=TRUE -DLORE_BUILD_GUEST_TARGETS=FALSE -DLORE_BUILD_TESTS=OFF
    cmake --build "$REPOS_DIR/build/staging" --target install
fi

# --- 3. bundle the target-arch clang/LLVM ----------------------------------------------------------
# For the native arch this is /usr/lib/llvm-N; for a cross target it is the same path populated with
# the target-arch debs by bootstrap-dist.sh (dpkg-deb extracted, so the binaries are target-arch).
"$SCRIPT_DIR/bundle-llvm.sh" "$TREE" "$LLVM_VER" "$LLVM_SRC"

# --- 4. x86_64 guest sysroot (self-contained), under x86_64/sysroot/ so it stays separate from the
# guest lorelei install in x86_64/lib/ (which lets the runtime cut drop the sysroot cleanly). The
# stable thunks wrap zlib and lzma, so their amd64 dev + runtime go into the guest build environment. --
"$SCRIPT_DIR/make-sysroot.sh" "$TREE/x86_64/sysroot" "$GCC_VER" \
    zlib1g-dev:amd64 zlib1g:amd64 liblzma-dev:amd64 liblzma5:amd64

# --- 5. guest cross cmake file + wrappers (for the end user on the target) --------------------------
mkdir -p "$TREE/share/lorelei/toolchains"
cp "$TOOLCHAIN_DIR/x86_64-linux-gnu.cmake" "$TREE/share/lorelei/toolchains/x86_64-linux-gnu.cmake"
"$SCRIPT_DIR/write-wrappers.sh" "$TREE"

# A build-time x86_64 guest toolchain using this builder's native clang (the bundled clang in $TREE is
# the target arch on a cross build and cannot run here). The shipped guest toolchain file in step 5 is
# the end user's; this one is only for building the guest side of the tree.
GUEST_TC="$REPOS_DIR/guest-build-$TARGET.cmake"
cat > "$GUEST_TC" <<EOF
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET x86_64-linux-gnu)
set(CMAKE_CXX_COMPILER_TARGET x86_64-linux-gnu)
set(CMAKE_ASM_COMPILER_TARGET x86_64-linux-gnu)
set(CMAKE_SYSROOT $TREE/x86_64/sysroot)
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_FIND_ROOT_PATH "$TREE/x86_64/sysroot")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
EOF

# --- 6. lorelei guest side (x86_64, native clang on this builder): LoreGuestRT ----------------------
cmake -B "$REPOS_DIR/build/$TARGET-guest" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$GUEST_TC" \
    -DCMAKE_INSTALL_PREFIX="$TREE/x86_64" \
    -Dqmsetup_DIR="$QMSETUP_NATIVE" \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE \
    -DLORE_BUILD_TESTS=OFF
cmake --build "$REPOS_DIR/build/$TARGET-guest" --target install

# --- 7. thunks: host HTL (target) + guest GTL (x86_64) ---------------------------------------------
# The guest generate parse targets x86_64 against the guest sysroot for its headers.
gtl_gen_args="--target=x86_64-linux-gnu;--sysroot=$TREE/x86_64/sysroot"
if [ "$CROSS" = "0" ]; then
    # Native: the host build runs the in-tree (native) TLC to generate both sources and install the
    # HTL; the guest build then reuses that generated source.
    cmake -S "$THUNKS_SRC" -B "$REPOS_DIR/build/$TARGET-thunk-host" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TREE" \
        -Dqmsetup_DIR="$QMSETUP_NATIVE" -Dlorelei_DIR="$TREE/lib/cmake/lorelei" \
        -DTHUNK_GTL_EXTRA_ARGS="$gtl_gen_args" \
        -DTHUNK_BUILD_HOST_TARGETS=TRUE -DTHUNK_BUILD_GUEST_TARGETS=FALSE
    cmake --build "$REPOS_DIR/build/$TARGET-thunk-host" --target install
    gen_src="$TREE/share/lorelei/thunks"
else
    # Cross: the in-tree TLC is target-arch and cannot run here, so generate both sources with the
    # native staging TLC targeting $TARGET (its host parse points at the cross toolchain headers), then
    # cross-compile the HTL from the generated source.
    gen="$REPOS_DIR/build/$TARGET-thunk-gen"
    cmake -S "$THUNKS_SRC" -B "$gen" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$gen/install" \
        -Dqmsetup_DIR="$QMSETUP_NATIVE" -Dlorelei_DIR="$STAGING/lib/cmake/lorelei" \
        -DTHUNK_HOST_ARCH="$TARGET" \
        -DTHUNK_HTL_EXTRA_ARGS="--gcc-toolchain=/usr;-idirafter;/usr/include" \
        -DTHUNK_GTL_EXTRA_ARGS="$gtl_gen_args" \
        -DTHUNK_BUILD_HOST_TARGETS=FALSE -DTHUNK_BUILD_GUEST_TARGETS=FALSE
    cmake --build "$gen" --target install
    gen_src="$gen/install/share/lorelei/thunks"
    cmake -S "$THUNKS_SRC" -B "$REPOS_DIR/build/$TARGET-thunk-host" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TREE" \
        -Dqmsetup_DIR="$QMSETUP_NATIVE" -Dlorelei_DIR="$TREE/lib/cmake/lorelei" \
        -DTHUNK_GEN_SOURCE_DIR="$gen_src" \
        -DTHUNK_BUILD_HOST_TARGETS=TRUE -DTHUNK_BUILD_GUEST_TARGETS=FALSE \
        "${host_toolchain[@]}"
    cmake --build "$REPOS_DIR/build/$TARGET-thunk-host" --target install
fi

# guest GTL (x86_64, native clang) from the generated source, installed under x86_64/
cmake -S "$THUNKS_SRC" -B "$REPOS_DIR/build/$TARGET-thunk-guest" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$GUEST_TC" \
    -DCMAKE_INSTALL_PREFIX="$TREE/x86_64" \
    -Dqmsetup_DIR="$QMSETUP_NATIVE" \
    -Dlorelei_DIR="$TREE/x86_64/lib/cmake/lorelei" \
    -DTHUNK_GEN_SOURCE_DIR="$gen_src" \
    -DTHUNK_BUILD_HOST_TARGETS=FALSE -DTHUNK_BUILD_GUEST_TARGETS=TRUE
cmake --build "$REPOS_DIR/build/$TARGET-thunk-guest" --target install

# --- 8. cut the two tarballs -----------------------------------------------------------------------
"$SCRIPT_DIR/cut-tarballs.sh" "$TREE" "$TARGET" "$OUT_DIR" "$LLVM_VER"

# --- 9. reclaim disk -------------------------------------------------------------------------------
# Only $OUT_DIR needs to survive. When several arches share one docker RUN (the deploy build), the
# per-target build dirs, the packed tree and the extracted target LLVM would otherwise accumulate into
# the final image layer and can exhaust the builder's disk. The native staging TLC is reused across
# arches, so keep its install; drop everything target-specific.
rm -rf "$REPOS_DIR/build/$TARGET-host" "$REPOS_DIR/build/$TARGET-thunk-host" \
       "$REPOS_DIR/build/$TARGET-thunk-guest" "$REPOS_DIR/build/$TARGET-thunk-gen" \
       "$REPOS_DIR/dist/$TARGET"
if [ "$CROSS" = "1" ]; then
    rm -rf "/opt/xllvm/$TARGET"
    apt-get clean 2>/dev/null || true
fi

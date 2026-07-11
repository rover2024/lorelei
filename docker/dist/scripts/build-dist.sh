#!/usr/bin/env bash
#
# Build the lorelei distribution for one target arch and cut three tarballs from a single tree: a small
# runtime (for running thunked guests), a full devkit (runtime + devel + toolchain, for building
# thunks), and a thunks pack (the prebuilt thunks alone). All use the Scheme A layout: the host-arch
# side at the prefix root and the x86_64 guest side nested under x86_64/.
#
#   <tree>/                 host runtime + devel + LoreTLC + host thunks (HTL) + bundled clang/LLVM
#   <tree>/x86_64/          x86_64 guest runtime + devel + guest thunks (GTL) + guest sysroot
#
# runtime-<arch>.tar.xz  = the lorelei runtime .so's alone (no toolchain, no thunks).
# devkit-<arch>.tar.xz   = the whole tree minus the thunks (toolchain + runtime + headers/sysroot).
# thunks-<arch>.tar.xz   = the prebuilt thunks (HTL/GTL + ThunkDB.json), a drop-in thunk pack.
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
NATIVE_LLVM_SRC="/opt/lore-llvm/x86_64"
LLVM_SRC="$NATIVE_LLVM_SRC"   # our self-contained native LLVM; bundle-llvm.sh bundles this
if [ "$TARGET" != "$build_arch" ]; then
    CROSS=1
    # Install the cross gcc, fetch the target-arch clang/LLVM into a prefix and the thunk libs into
    # the multiarch tree. The target LLVM cannot live at /opt/lore-llvm/x86_64 (the native x86_64 clang
    # does), so link it dynamically from the prefix.
    "$SCRIPT_DIR/prepare-cross.sh" "$TARGET"
    LLVM_SRC="/opt/lore-llvm/$TARGET"
    triplet="$TARGET-linux-gnu"
    # Our libLLVM.so / libclang-cpp.so are self-contained (no libedit/xml2/ffi to chase), so the cross
    # link only needs to find those two .so's themselves; point -rpath-link at the target prefix's lib.
    link_flags="-Wl,-rpath-link,$LLVM_SRC/lib"
    host_toolchain=(
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/$TARGET-linux-gnu.cmake"
        -DCMAKE_EXE_LINKER_FLAGS="$link_flags"
        -DCMAKE_SHARED_LINKER_FLAGS="$link_flags"
    )
fi
# LoreTLC links our self-contained Clang/LLVM (libclang-cpp.so + libLLVM.so) for every target: native
# from /opt/lore-llvm/x86_64, cross from the target prefix prepare-cross.sh fetched.
host_toolchain+=(
    -DClang_DIR="$LLVM_SRC/lib/cmake/clang"
    -DLLVM_DIR="$LLVM_SRC/lib/cmake/llvm"
    -DLORE_STATIC_LLVM=OFF
)

# --- 1. qmsetup (build tool, native x86_64) --------------------------------------------------------
# Used at build time by the lorelei/thunks builds, and shipped in the devkit because the end user's
# thunk build does find_package(qmsetup). One native x86_64 qmsetup serves both roles for every target:
# the thunk build only uses qmsetup's cmake helpers (qm_import, qm_basic_install), never runs qmcorecmd,
# so the tool's arch is irrelevant and no cross build is needed.
INSTALL_DIR="$REPOS_DIR/qmsetup-native" bash "$COMMON_SCRIPTS/build-qmsetup.sh"
QMSETUP_NATIVE="$REPOS_DIR/qmsetup-native/lib/cmake/qmsetup"
# Install a second copy into the devkit tree (the runtime cut drops lib/cmake, so this rides only in
# the devkit). Reuses the build dir build-qmsetup.sh left in $REPOS_DIR/qmsetup.
cmake --install "$REPOS_DIR/qmsetup/build" --prefix "$TREE" >/dev/null

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
if [ "$CROSS" = "1" ]; then
    if [ ! -x "$STAGING/bin/LoreTLC" ]; then
        cmake -B "$REPOS_DIR/build/staging" -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX="$STAGING" \
            -Dqmsetup_DIR="$QMSETUP_NATIVE" \
            -DClang_DIR="$NATIVE_LLVM_SRC/lib/cmake/clang" \
            -DLLVM_DIR="$NATIVE_LLVM_SRC/lib/cmake/llvm" \
            -DLORE_BUILD_TOOLS=TRUE -DLORE_BUILD_GUEST_TARGETS=FALSE -DLORE_BUILD_TESTS=OFF
        cmake --build "$REPOS_DIR/build/staging" --target install
    fi
    # The staging TLC (native) generates cross thunk sources, but it is not run through bundle-llvm.sh.
    # Give it the native clang resource headers at the path LibTooling resolves from its own binary.
    mkdir -p "$STAGING/lib"
    ln -sfnT "$NATIVE_LLVM_SRC/lib/clang" "$STAGING/lib/clang"
fi

# --- 3. bundle the target-arch clang/LLVM ----------------------------------------------------------
# Our self-contained prefix for this arch: /opt/lore-llvm/x86_64 on a native build, or the target-arch
# prefix prepare-cross.sh fetched. bundle-llvm.sh copies it into the tree and trims it to the runtime.
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
    -DLORE_BUILD_HOST_TARGETS=FALSE \
    -DLORE_BUILD_TESTS=OFF
cmake --build "$REPOS_DIR/build/$TARGET-guest" --target install

# --- 6b. bundle the C++ runtime so the devkit is self-contained -------------------------------------
# LoreHostRT + host thunks (host arch) and LoreGuestRT + guest thunks (x86_64) link libstdc++/libgcc_s
# dynamically. Ship the shared libraries next to each runtime, so a thunk needs only libc from the
# machine it runs on: the host copy in lib/, the x86_64 copy in x86_64/lib/, both already on the
# LD_LIBRARY_PATH the run sets. Each is located with -print-file-name so it matches the compiler that
# built that side (the cross g++ on a cross host, the guest clang's sysroot on the x86_64 side).
bundle_cxx_runtime() {  # <compiler> <dest-lib-dir> [extra compiler args...]
    local cxx="$1" dest="$2"; shift 2
    local lib src
    for lib in libstdc++.so.6 libgcc_s.so.1; do
        src="$("$cxx" "$@" -print-file-name="$lib")"
        [ -f "$src" ] || { echo "[build-dist] $lib not found via $cxx"; exit 1; }
        cp -L "$src" "$dest/$lib"
        # linker name (libstdc++.so -> libstdc++.so.6) so the host thunk's clang++ can `-lstdc++`
        # against the bundled runtime without a system libstdc++-dev present.
        ln -sf "$lib" "$dest/${lib%.*}"
    done
}

# The libstdc++ C++ headers too, so a build host needs a C compiler (for the libc headers) but not
# g++ / libstdc++-dev. Copy the host compiler's C++ system-include dirs, in order, into lib/cxx/0, /1,
# ...; LoreMakeThunk feeds them to the host parse/compile with -nostdinc++. (The guest side already
# has its C++ headers inside x86_64/sysroot, from make-sysroot.)
bundle_cxx_headers() {  # <compiler>
    local cxx="$1" i=0 d
    rm -rf "$TREE/lib/cxx"; mkdir -p "$TREE/lib/cxx"
    while IFS= read -r d; do
        [ -d "$d" ] || continue
        cp -a "$d" "$TREE/lib/cxx/$i"
        i=$((i + 1))
    done < <("$cxx" -xc++ -E -Wp,-v - </dev/null 2>&1 | sed -n 's/^ \(\/[^ ]*\)$/\1/p' | grep '/c++/')
    [ "$i" -gt 0 ] || { echo "[build-dist] no C++ headers found via $cxx"; exit 1; }
}

host_cxx=g++
[ "$CROSS" = "1" ] && host_cxx="$TARGET-linux-gnu-g++"
bundle_cxx_runtime "$host_cxx" "$TREE/lib"
bundle_cxx_runtime clang++ "$TREE/x86_64/lib" --target=x86_64-linux-gnu --sysroot="$TREE/x86_64/sysroot"
bundle_cxx_headers "$host_cxx"

# --- 7. thunks: host HTL (target) + guest GTL (x86_64) ---------------------------------------------
# The guest generate parse targets x86_64 against the guest sysroot for its headers.
gtl_gen_args="--target=x86_64-linux-gnu;--sysroot=$TREE/x86_64/sysroot"
if [ "$CROSS" = "0" ]; then
    # Native: the host build runs the in-tree (native) TLC to generate both sources and install the
    # HTL; the guest build then reuses that generated source.
    cmake -S "$THUNKS_SRC" -B "$REPOS_DIR/build/$TARGET-thunk-host" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$TREE" \
        -Dqmsetup_DIR="$QMSETUP_NATIVE" -Dlorelei_DIR="$TREE/lib/cmake/lorelei" \
        -DTHUNK_GTL_TLC_OPTIONS="$gtl_gen_args" \
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
        -DTHUNK_HTL_TLC_OPTIONS="--gcc-toolchain=/usr;-idirafter;/usr/include" \
        -DTHUNK_GTL_TLC_OPTIONS="$gtl_gen_args" \
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
    rm -rf "/opt/lore-llvm/$TARGET"
    apt-get clean 2>/dev/null || true
fi

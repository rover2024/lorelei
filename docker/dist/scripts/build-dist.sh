#!/usr/bin/env bash
#
# Build the lorelei distribution for one target arch and cut two tarballs from a single tree: a small
# runtime (for running thunked guests) and a full devkit (runtime + devel + toolchain, for building
# thunks). All use the Scheme A layout: the host-arch side at the prefix root and the x86_64 guest
# side nested under x86_64/.
#
#   <tree>/                 host runtime + devel + LoreTLC + bundled clang/LLVM
#   <tree>/x86_64/          x86_64 guest runtime + devel + guest sysroot
#
# runtime-<arch>.tar.xz  = the lorelei runtime .so's alone (no toolchain).
# devkit-<arch>.tar.xz   = the whole tree (toolchain + runtime + headers/sysroot).
#
# No thunk is built here. Thunks are downstream: they build against the devkit this produces, so
# shipping them from this repository would point the dependency the wrong way.
#
# Everything is built on an x86_64 host: the native arch natively, and aarch64/riscv64 by cross
# compilation (the guest x86_64 side is always native here). No target binary runs at build time.
set -euo pipefail
: "${LORELEI_SRC:?}"    # lorelei v2 source tree
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

# --- 3. bundle the target-arch clang/LLVM ----------------------------------------------------------
# Our self-contained prefix for this arch: /opt/lore-llvm/x86_64 on a native build, or the target-arch
# prefix prepare-cross.sh fetched. bundle-llvm.sh copies it into the tree and trims it to the runtime.
"$SCRIPT_DIR/bundle-llvm.sh" "$TREE" "$LLVM_VER" "$LLVM_SRC"

# --- 4. x86_64 guest sysroot (self-contained), under x86_64/sysroot/ so it stays separate from the
# guest lorelei install in x86_64/lib/ (which lets the runtime cut drop the sysroot cleanly). zlib and
# lzma ride along so a guest thunk for either builds against the devkit with no extra setup. --------
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

# --- 6b. bundle the C++ runtime the two sides need --------------------------------------------------
# The guest thunks and LoreGuestRT link libstdc++/libgcc_s and run under qemu against the x86_64
# sysroot, which carries no libstdc++, so the x86_64 copy is shipped next to the guest runtime in
# x86_64/lib/ (on the guest LD_LIBRARY_PATH). The host side is different: every host we support has a
# system libstdc++ new enough (the devkit's own C++ needs only GLIBCXX <= 3.4.31), so LoreHostRT, the
# host thunks and clang all bind the host's system libstdc++ at run time, which keeps a thunked host
# C++ library on the host's own libstdc++ rather than a copy we ship. The host libstdc++/libgcc are
# kept only as a build-only copy in lib/cxx-link, off every runtime search path, so a host thunk can
# still link -lstdc++ without a system libstdc++-dev. Each is located with -print-file-name so it
# matches the compiler that built that side.
bundle_cxx_runtime() {  # <compiler> <dest-lib-dir> [extra compiler args...]
    local cxx="$1" dest="$2"; shift 2
    local lib src
    mkdir -p "$dest"
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
bundle_cxx_headers "$host_cxx"
bundle_cxx_runtime "$host_cxx" "$TREE/lib/cxx-link"    # host: build-only, for the host thunk -lstdc++
bundle_cxx_runtime clang++ "$TREE/x86_64/lib" --target=x86_64-linux-gnu --sysroot="$TREE/x86_64/sysroot"

# The config LoreMakeThunk reads: it names the tools, directories and flags above rather than letting
# the script assume a layout. Written here because this is where those paths are made, so the lib/cxx
# ordering and the presence of lib/cxx-link are recorded rather than probed. Every path is relative to
# ${root}, itself relative to the config's own directory, so the unpacked devkit is relocatable.
write_makethunk_config() {
    local cfg="$TREE/share/lorelei/MakeThunkConfig.json" d
    mkdir -p "$(dirname "$cfg")"

    # An optional tool is named only when the devkit actually ships it, else null so the script falls
    # back to the one on PATH. Naming a file that is not there is a hard error, by design: the config
    # is meant to describe this tree, so a wrong path should be caught rather than silently ignored.
    opt_tool() {  # <key> <relative path>
        if [ -x "$TREE/$2" ]; then printf '"%s": "${root}/%s"' "$1" "$2"
        else printf '"%s": null' "$1"; fi
    }

    # -nostdinc++ plus the bundled C++ header dirs, in the order the host compiler reported them.
    local isystem='"-nostdinc++"'
    for d in "$TREE"/lib/cxx/*/; do
        [ -d "$d" ] || continue
        isystem="$isystem, \"-isystem\", \"\${root}/lib/cxx/$(basename "$d")\""
    done

    cat > "$cfg" <<JSON
{
    "\$vars": {
        "root": "\${configDir}/../.."
    },
    "tools": {
        "tlc": "\${root}/bin/LoreTLC",
        "host_cxx": "\${root}/bin/clang++",
        "guest_cxx": "\${root}/bin/x86_64-linux-gnu-clang++",
        $(opt_tool nm bin/llvm-nm),
        $(opt_tool readelf bin/llvm-readelf)
    },
    "host": {
        "include": "\${root}/include",
        "libdir": "\${root}/lib",
        "cxx_flags": [$isystem],
        "link_flags": ["-L\${root}/lib/cxx-link"]
    },
    "guest": {
        "include": "\${root}/x86_64/include",
        "libdir": "\${root}/x86_64/lib",
        "triplet": "x86_64-pc-linux-gnu",
        "cxx_flags": ["--sysroot=\${root}/x86_64/sysroot"]
    }
}
JSON
}
write_makethunk_config

# --- 7. cut the two tarballs -----------------------------------------------------------------------
"$SCRIPT_DIR/cut-tarballs.sh" "$TREE" "$TARGET" "$OUT_DIR" "$LLVM_VER"

# --- 8. reclaim disk -------------------------------------------------------------------------------
# Only $OUT_DIR needs to survive. When several arches share one docker RUN (the deploy build), the
# per-target build dirs, the packed tree and the extracted target LLVM would otherwise accumulate into
# the final image layer and can exhaust the builder's disk.
rm -rf "$REPOS_DIR/build/$TARGET-host" "$REPOS_DIR/dist/$TARGET"
if [ "$CROSS" = "1" ]; then
    rm -rf "/opt/lore-llvm/$TARGET"
    apt-get clean 2>/dev/null || true
fi

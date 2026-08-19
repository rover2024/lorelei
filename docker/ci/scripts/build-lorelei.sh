#!/usr/bin/env bash
#
# Build this repository (lorelei v2) from the copied-in source, twice, into a uniform layout. Both
# sides share one flat prefix (no per-repo subdir), alongside qmsetup:
#   install         host side: LoreTLC + LoreHostRT (this machine's arch), and the tests
#   install/x86_64  guest side: LoreGuestRT (x86_64)
#
# Both builds are the same on every host: the host one is native and host-only, the guest one is
# x86_64 (native on an x86_64 host, cross elsewhere). The end-to-end test builds its two sides from
# these two prefixes through LoreMakeThunk, so no build tree has to hold both.
set -euo pipefail
: "${LORELEI_SRC:?}"
: "${INSTALL_DIR:?}"

# Host toolchain: gcc by default, clang for the LLVM CI variant (LORE_TOOLCHAIN=llvm). Only the host
# side switches, so the x86_64 guest cross toolchain below is unaffected.
case "${LORE_TOOLCHAIN:-gcc}" in
    llvm) HOST_CC=clang-20; HOST_CXX=clang++-20 ;;
    *)    HOST_CC=gcc;      HOST_CXX=g++ ;;
esac

arch="$(uname -m)"
guest_extra=()
host_extra=()
# Flags that make the x86_64 headers reachable: to the TLC guest parse (always clang) and to the
# guest compiler. Empty where the cross toolchain is found without help.
guest_parse=()
guest_cxx_flags=()
if [ "$arch" = "x86_64" ] || [ "$arch" = "amd64" ]; then
    GUEST_CC=gcc
    GUEST_CXX=g++
elif [ "$arch" = "riscv64" ]; then
    # Our Canadian-cross toolchain (gcc 11.4) carries its own sysroot. Point it at the amd64
    # multiarch ffcall: -idirafter keeps avcall.h from shadowing the toolchain's own libc headers,
    # and the extra -L finds libavcall.
    GUEST_CC=x86_64-unknown-linux-gnu-gcc
    GUEST_CXX=x86_64-unknown-linux-gnu-g++
    guest_extra=(
        "-DCMAKE_C_FLAGS=-idirafter /usr/include"
        "-DCMAKE_CXX_FLAGS=-idirafter /usr/include"
        "-DCMAKE_EXE_LINKER_FLAGS=-L/usr/lib/x86_64-linux-gnu"
        "-DCMAKE_SHARED_LINKER_FLAGS=-L/usr/lib/x86_64-linux-gnu"
    )
    # The host build's TLC generates the x86_64 guest test fixture; clang cannot find x86_64 C++
    # headers natively here, so point its guest parse at the toolchain (--gcc-toolchain / --sysroot),
    # with -idirafter /usr/include as the fallback for any non-toolchain header.
    tc=/opt/x86_64-unknown-linux-gnu
    guest_parse=( "--gcc-toolchain=$tc" "--sysroot=$tc/x86_64-unknown-linux-gnu/sysroot" "-idirafter" "/usr/include" )
    guest_cxx_flags=( "-idirafter" "/usr/include" )
    host_extra=( "-DLORE_TLC_GUEST_EXTRA_ARGS=--gcc-toolchain=$tc;--sysroot=$tc/x86_64-unknown-linux-gnu/sysroot;-idirafter;/usr/include" )
else
    GUEST_CC=x86_64-linux-gnu-gcc
    GUEST_CXX=x86_64-linux-gnu-g++
fi

cd "$LORELEI_SRC"

# Host side (native), with the tests. The build tree is kept so the test run can drive it.
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$HOST_CC" \
    -DCMAKE_CXX_COMPILER="$HOST_CXX" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -Dqmsetup_DIR="$INSTALL_DIR/lib/cmake/qmsetup" \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=FALSE \
    -DLORE_BUILD_TESTS=ON \
    "${host_extra[@]}"
cmake --build build --target all
cmake --build build --target install

# Guest side: the x86_64 runtime, installed under install/x86_64.
cmake -B build-guest -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$GUEST_CC" \
    -DCMAKE_CXX_COMPILER="$GUEST_CXX" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/x86_64" \
    -Dqmsetup_DIR="$INSTALL_DIR/lib/cmake/qmsetup" \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE \
    -DLORE_BUILD_HOST_TARGETS=FALSE \
    "${guest_extra[@]}"
cmake --build build-guest --target install

# The config LoreMakeThunk reads. This tree is not a devkit (no bundled toolchain, no guest sysroot),
# so rather than assemble one, name what is actually here: the system compilers, the two install
# prefixes, and the flags that reach the x86_64 headers.
json_list() {  # emit the arguments as a JSON string array
    local out="" x
    for x in "$@"; do out="$out${out:+, }\"$x\""; done
    printf '[%s]' "$out"
}
mkdir -p "$INSTALL_DIR/share/lorelei"
cat > "$INSTALL_DIR/share/lorelei/MakeThunkConfig.json" <<JSON
{
    "\$vars": {
        "root": "\${configDir}/../.."
    },
    "tools": {
        "tlc": "\${root}/bin/LoreTLC",
        "host_cxx": "$(command -v "$HOST_CXX")",
        "guest_cxx": "$(command -v "$GUEST_CXX")"
    },
    "host": {
        "include": "\${root}/include",
        "libdir": "\${root}/lib"
    },
    "guest": {
        "include": "\${root}/x86_64/include",
        "libdir": "\${root}/x86_64/lib",
        "triplet": "x86_64-pc-linux-gnu",
        "parse_flags": $(json_list "${guest_parse[@]+"${guest_parse[@]}"}"),
        "cxx_flags": $(json_list "${guest_cxx_flags[@]+"${guest_cxx_flags[@]}"}")
    }
}
JSON

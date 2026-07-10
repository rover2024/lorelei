#!/usr/bin/env bash
#
# Install what is needed to BUILD the lorelei distribution on an x86_64 host: a native clang/LLVM (for
# the native-arch build and for building the x86_64 guest side), CMake/Ninja, and the native lorelei
# host dependencies. Cross targets additionally add their toolchain and target-arch LLVM on demand via
# prepare-cross.sh; this only sets up the pieces shared by every target.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"
LLVM_VER="${LLVM_VER:-20}"

echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries

if [ "$USE_USTC_MIRROR" = "1" ]; then
    sed -i -E 's@https?://(archive|security)\.ubuntu\.com/ubuntu@http://mirrors.ustc.edu.cn/ubuntu@g' \
        /etc/apt/sources.list.d/ubuntu.sources
fi

apt-get update
apt-get install -y --no-install-recommends \
    ca-certificates git curl wget xz-utils file patchelf pv \
    cmake ninja-build python3 pkg-config g++

# Native Clang/LLVM: our self-contained build (see fetch-llvm.sh), not the distro debs. It is the
# native compiler that builds the x86_64 guest side and generates the thunks, it is linked into
# LoreTLC, and it is bundled into the x86_64 devkit. Putting its lib dir on the loader path lets the
# freshly built LoreTLC and the bundled clang resolve libLLVM.so / libclang-cpp.so here, and its clang
# is exposed on PATH for the guest toolchain (CMAKE_C_COMPILER=clang).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
bash "$SCRIPT_DIR/fetch-llvm.sh" x86_64 /opt/lore-llvm/x86_64
echo /opt/lore-llvm/x86_64/lib > /etc/ld.so.conf.d/lore-llvm.conf
ldconfig
for tool in clang clang++ clang-cpp lld ld.lld llvm-ar llvm-nm llvm-objcopy llvm-strip; do
    ln -sf "/opt/lore-llvm/x86_64/bin/$tool" "/usr/local/bin/$tool"
done

# Native lorelei host dependency: ffcall backs the VariadicAdaptor in LoreHostRT. zlib/lzma dev provide
# the headers and link libraries for the stable thunks the distribution ships. (Cross targets add the
# target-arch versions of these via prepare-cross.sh.)
apt-get install -y --no-install-recommends libffcall-dev zlib1g-dev liblzma-dev

echo "[bootstrap-dist] done"

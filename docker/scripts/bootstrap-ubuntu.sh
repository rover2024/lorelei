#!/usr/bin/env bash
#
# Install the build toolchain and create the unprivileged build user. Targets an x86_64 (amd64)
# Ubuntu 22.04 base image: the guest and host are both x86_64, so one native toolchain builds
# everything.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"

if [ "$USE_USTC_MIRROR" = "1" ]; then
    sed -i 's@http://archive.ubuntu.com/ubuntu/@http://mirrors.ustc.edu.cn/ubuntu/@g' /etc/apt/sources.list
    sed -i 's@http://security.ubuntu.com/ubuntu/@http://mirrors.ustc.edu.cn/ubuntu/@g' /etc/apt/sources.list
fi

apt-get update

# Basic build tools.
apt-get install -y --no-install-recommends \
    sudo ca-certificates git curl wget vim less file \
    build-essential gcc g++ gdb cmake ninja-build pkg-config \
    python3 python3-pip meson flex bison patchelf

# Clang/LLVM 20: TLC (the thunk compiler) is built on Clang LibTooling. Installed from the official
# apt.llvm.org script; libclang-20-dev provides the ClangConfig.cmake that find_package(Clang) needs.
apt-get install -y --no-install-recommends lsb-release software-properties-common gnupg
wget https://apt.llvm.org/llvm.sh -O /tmp/llvm.sh
chmod +x /tmp/llvm.sh
/tmp/llvm.sh 20
apt-get install -y --no-install-recommends libclang-20-dev

# Lorelei build/runtime dependencies: ffcall backs the VariadicAdaptor, Boost.Test backs the auto
# tests, and zlib is what the example thunk wraps. minizip provides the unmodified guest binary the
# end-to-end test runs over the zlib thunk.
apt-get install -y --no-install-recommends \
    libffcall-dev libboost-test-dev zlib1g-dev minizip

# QEMU build dependencies.
apt-get install -y --no-install-recommends \
    python3-tomli libglib2.0-dev libpixman-1-dev libfdt-dev \
    libslirp-dev libcapstone-dev libffi-dev libelf-dev libssl-dev

# On a non-amd64 host the guest side (always x86_64) is cross-compiled and runs under qemu-x86_64, so
# pull in the x86_64 cross-toolchain, the amd64 runtime libraries, and an x86_64 minizip binary. On an
# amd64 host the native toolchain and minizip already serve both sides, so this is skipped.
host_arch="$(dpkg --print-architecture)"
if [ "$host_arch" != "amd64" ]; then
    dpkg --add-architecture amd64
    codename="$(. /etc/os-release && echo "$VERSION_CODENAME")"
    # The non-amd64 base image serves packages from ubuntu-ports; pin those to the native arch and add
    # the regular archive for amd64.
    sed -i -E "/ubuntu-ports/ s/^deb /deb [arch=${host_arch}] /" /etc/apt/sources.list
    cat >/etc/apt/sources.list.d/amd64.list <<EOF
deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ ${codename} main restricted universe multiverse
deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ ${codename}-updates main restricted universe multiverse
deb [arch=amd64] http://archive.ubuntu.com/ubuntu/ ${codename}-security main restricted universe multiverse
EOF
    apt-get update
    apt-get install -y --no-install-recommends \
        gcc-x86-64-linux-gnu g++-x86-64-linux-gnu \
        libc6:amd64 libstdc++6:amd64 zlib1g:amd64 libminizip1:amd64

    # Unpack the amd64 minizip CLI and expose it as minizip-x86_64 (the guest binary the test runs).
    tmp="$(mktemp -d)"
    ( cd "$tmp" && apt-get download minizip:amd64 )
    dpkg-deb -x "$tmp"/minizip_*_amd64.deb /opt/minizip-amd64
    ln -sf /opt/minizip-amd64/usr/bin/minizip /usr/local/bin/minizip-x86_64
    rm -rf "$tmp"

    # Make sure the x86_64 dynamic loader is found at its canonical path for running x64 binaries.
    if [ ! -e /lib64/ld-linux-x86-64.so.2 ] && [ -e /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 ]; then
        mkdir -p /lib64
        ln -s /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 /lib64/ld-linux-x86-64.so.2
    fi
fi

# Unprivileged build user.
if ! id -u user >/dev/null 2>&1; then
    groupadd --gid 1000 user || true
    useradd --uid 1000 --gid 1000 -m -s /bin/bash user
    echo "user:123456" | chpasswd
    usermod -aG sudo user
    echo "user ALL=(ALL) NOPASSWD:ALL" >/etc/sudoers.d/user
    chmod 0440 /etc/sudoers.d/user
fi

echo "[bootstrap] done"

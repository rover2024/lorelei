#!/usr/bin/env bash
#
# Install the build toolchain and create the unprivileged build user. Targets an x86_64 (amd64)
# Ubuntu 24.04 base image: the guest and host are both x86_64, so one native toolchain builds
# everything.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
USE_USTC_MIRROR="${USE_USTC_MIRROR:-0}"

# Retry transient fetch failures (e.g. a proxy returning 502) instead of aborting the whole build.
echo 'Acquire::Retries "5";' > /etc/apt/apt.conf.d/80-retries

# Ubuntu 24.04 keeps its apt sources in the deb822 file /etc/apt/sources.list.d/ubuntu.sources.
if [ "$USE_USTC_MIRROR" = "1" ]; then
    sed -i -E 's@https?://(archive|security)\.ubuntu\.com/ubuntu@http://mirrors.ustc.edu.cn/ubuntu@g; s@https?://ports\.ubuntu\.com/ubuntu-ports@http://mirrors.ustc.edu.cn/ubuntu-ports@g' \
        /etc/apt/sources.list.d/ubuntu.sources
fi

apt-get update

# Basic build tools.
apt-get install -y --no-install-recommends \
    sudo ca-certificates git curl wget vim less file \
    build-essential gcc g++ gdb cmake ninja-build pkg-config \
    python3 python3-pip meson flex bison patchelf

# Clang/LLVM 20: TLC (the thunk compiler) is built on Clang LibTooling. Ubuntu 24.04 ships LLVM 20 in
# its own repositories, so install it straight from apt. find_package(Clang) needs ClangConfig.cmake
# (from clang-20) and LLVMConfig.cmake (from llvm-20-dev); libclang-20-dev provides the Clang
# libraries and headers TLC links against.
apt-get install -y --no-install-recommends clang-20 llvm-20-dev libclang-20-dev

# Lorelei build/runtime dependencies: ffcall backs the VariadicAdaptor, Boost.Test backs the auto
# tests, and zlib / liblzma are what the built thunks wrap. minizip and xz provide the unmodified
# guest binaries the end-to-end test runs over the zlib and lzma thunks.
apt-get install -y --no-install-recommends \
    libffcall-dev libboost-test-dev zlib1g-dev minizip liblzma-dev xz-utils

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
    # The non-amd64 base serves packages from ports (deb822 ubuntu.sources); pin those entries to the
    # native arch and add a separate amd64 archive. Not yet validated on the cross hosts.
    sed -i "/^Types: deb/a Architectures: ${host_arch}" /etc/apt/sources.list.d/ubuntu.sources
    amd64_mirror="http://archive.ubuntu.com/ubuntu"
    [ "$USE_USTC_MIRROR" = "1" ] && amd64_mirror="http://mirrors.ustc.edu.cn/ubuntu"
    cat >/etc/apt/sources.list.d/amd64.sources <<EOF
Types: deb
URIs: ${amd64_mirror}
Suites: ${codename} ${codename}-updates ${codename}-security
Components: main restricted universe multiverse
Architectures: amd64
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOF
    apt-get update
    # The amd64 runtime/dev libraries the guest side links and runs against: libffcall (avcall, the
    # VariadicAdaptor backend) for the build, and libc/libstdc++/zlib/minizip/liblzma for running the
    # x86_64 guest binaries under qemu-x86_64.
    apt-get install -y --no-install-recommends \
        libffcall-dev:amd64 \
        libc6:amd64 libstdc++6:amd64 zlib1g:amd64 libminizip1:amd64 liblzma5:amd64

    # The x86_64 cross-compiler that builds the guest side.
    if [ "$host_arch" = "riscv64" ]; then
        # The riscv64 archive has no x86_64 cross-gcc, so fetch our prebuilt Canadian-cross toolchain
        # (runs on riscv64, targets x86_64, gcc 11.4) and expose it as x86_64-unknown-linux-gnu-*. It
        # carries its own sysroot; the guest build points it at the amd64 multiarch ffcall with flags.
        curl -fsSL https://github.com/rover2024/x86_64-riscv64-toolchain/releases/download/gcc-11.4.0-glibc-2.35/x86_64-unknown-linux-gnu-gcc11.4.0-glibc2.35-riscv64host.tar.xz \
            | tar -xJ -C /opt
        ln -sf /opt/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-* /usr/local/bin/
    else
        apt-get install -y --no-install-recommends gcc-x86-64-linux-gnu g++-x86-64-linux-gnu
    fi

    # Unpack the amd64 minizip CLI and expose it as minizip-x86_64 (the guest binary the test runs).
    tmp="$(mktemp -d)"
    ( cd "$tmp" && apt-get download minizip:amd64 )
    dpkg-deb -x "$tmp"/minizip_*_amd64.deb /opt/minizip-amd64
    ln -sf /opt/minizip-amd64/usr/bin/minizip /usr/local/bin/minizip-x86_64
    rm -rf "$tmp"

    # Likewise the amd64 xz CLI, exposed as xz-x86_64 (the guest binary for the lzma thunk test).
    tmp="$(mktemp -d)"
    ( cd "$tmp" && apt-get download xz-utils:amd64 )
    dpkg-deb -x "$tmp"/xz-utils_*_amd64.deb /opt/xz-utils-amd64
    ln -sf /opt/xz-utils-amd64/usr/bin/xz /usr/local/bin/xz-x86_64
    rm -rf "$tmp"

    # Make sure the x86_64 dynamic loader is found at its canonical path for running x64 binaries.
    if [ ! -e /lib64/ld-linux-x86-64.so.2 ] && [ -e /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 ]; then
        mkdir -p /lib64
        ln -s /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 /lib64/ld-linux-x86-64.so.2
    fi
fi

# Unprivileged build user.
if ! id -u user >/dev/null 2>&1; then
    # Ubuntu 24.04 ships a default 'ubuntu' user at UID/GID 1000; remove it so 'user' can claim 1000.
    userdel -r ubuntu 2>/dev/null || true
    groupdel ubuntu 2>/dev/null || true
    groupadd --gid 1000 user || true
    useradd --uid 1000 --gid 1000 -m -s /bin/bash user
    echo "user:123456" | chpasswd
    usermod -aG sudo user
    echo "user ALL=(ALL) NOPASSWD:ALL" >/etc/sudoers.d/user
    chmod 0440 /etc/sudoers.d/user
fi

echo "[bootstrap] done"

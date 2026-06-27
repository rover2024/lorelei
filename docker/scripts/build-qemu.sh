#!/usr/bin/env bash
#
# Build the patched QEMU and its dlcall plugin. The minimal-passthrough-plugin branch carries both
# the mmap/uaccess patch (so reserved_va == 0 and guest_base == 0, giving a shared address space) and
# contrib/plugins/dlcall.c, which builds into contrib/plugins/libdlcall.so when plugins are enabled.
set -euo pipefail
: "${REPOS_DIR:?}"

cd "$REPOS_DIR"
if [ ! -d qemu/.git ]; then
    git clone https://github.com/rover2024/qemu.git
fi
cd qemu
git checkout minimal-passthrough-plugin

mkdir -p build/release
cd build/release
../../configure --target-list=x86_64-linux-user --enable-plugins --python=python3
ninja

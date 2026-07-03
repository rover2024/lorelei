#!/usr/bin/env bash
#
# Build the patched QEMU and its dlcall plugin. The minimal-passthrough-plugin branch carries both
# the mmap/uaccess patch (so reserved_va == 0 and guest_base == 0, giving a shared address space) and
# contrib/plugins/dlcall.c, which builds into contrib/plugins/libdlcall.so when plugins are enabled.
# Fetched as a source tarball instead of cloned (much faster over a proxy): the roms submodules are
# not needed for linux-user, and the meson subprojects are downloaded at build time either way.
set -euo pipefail
: "${REPOS_DIR:?}"

cd "$REPOS_DIR"
if [ ! -d qemu ]; then
    curl -fsSL https://codeload.github.com/rover2024/qemu/tar.gz/refs/heads/minimal-passthrough-plugin | tar xz
    mv qemu-minimal-passthrough-plugin qemu
fi
cd qemu

mkdir -p build/release
cd build/release
../../configure --target-list=x86_64-linux-user --enable-plugins --python=python3
ninja

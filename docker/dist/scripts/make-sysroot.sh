#!/usr/bin/env bash
#
# Assemble a self-contained x86_64 sysroot (libc / libstdc++ / ffcall) from the amd64 debs, so the
# guest side needs no system x86_64 dev packages. Uses the debs (not the host's merged /usr) so it is
# correct on any build host.
set -euo pipefail
SYSROOT="$1"
GCC_VER="$2"
shift 2
EXTRA=("$@")   # extra amd64 packages, e.g. the thunked libraries' dev + runtime (zlib, lzma)

mkdir -p "$SYSROOT"
tmp="$(mktemp -d)"
(
    cd "$tmp"
    # libc/libstdc++ headers + libraries, plus ffcall (the guest DLCall links libavcall.a), plus any
    # extra packages the caller needs in the guest build environment.
    apt-get download \
        libc6:amd64 libc6-dev:amd64 linux-libc-dev:amd64 \
        "libgcc-s1:amd64" "libgcc-${GCC_VER}-dev:amd64" \
        "libstdc++6:amd64" "libstdc++-${GCC_VER}-dev:amd64" \
        libffcall-dev:amd64 "${EXTRA[@]}"
    for deb in *.deb; do dpkg-deb -x "$deb" "$SYSROOT"; done
)
rm -rf "$tmp"

# usrmerge compat: the noble debs are merged-/usr, but libc.so's linker script still names /lib and
# /lib64, which the linker reroots under the sysroot. Point them at usr/ so those resolve.
ln -s usr/lib "$SYSROOT/lib"
ln -s usr/lib64 "$SYSROOT/lib64"

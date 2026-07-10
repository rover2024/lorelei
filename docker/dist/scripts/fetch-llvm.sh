#!/usr/bin/env bash
#
# Fetch the self-contained Clang/LLVM toolchain for one host arch and extract it into <dest>. The
# distribution links LoreTLC against this LLVM and bundles its runtime .so's into the devkit, so the
# shipped toolchain depends only on glibc/libstdc++ and runs on any distro (unlike the distro LLVM
# debs, whose libLLVM.so pulls libedit/libxml2/libffi and fails on Fedora/Arch).
#
# The tarball unpacks to a single clang-<ver>-<arch>host/ directory holding a normal LLVM install
# prefix (bin/ lib/ include/); --strip-components=1 drops that so <dest> becomes the prefix root.
set -euo pipefail
ARCH="$1"                       # x86_64 | aarch64 | riscv64
DEST="$2"                       # prefix root to extract into
LLVM_VERSION="${LLVM_VERSION:-20.1.8}"
GLIBC_FLOOR="${GLIBC_FLOOR:-2.38}"
REPO="${LORE_LLVM_REPO:-rover2024/x86_64-riscv64-toolchain}"
TAG="${LORE_LLVM_TAG:-clang-${LLVM_VERSION}-glibc-${GLIBC_FLOOR}}"

file="clang${LLVM_VERSION}-glibc${GLIBC_FLOOR}-${ARCH}host.tar.xz"
url="https://github.com/${REPO}/releases/download/${TAG}/${file}"

tmp="$(mktemp -d)"
echo "[fetch-llvm] $ARCH <- $url"
ok=0
for i in 1 2 3 4 5; do
    if curl -fSL --retry 3 -o "$tmp/$file" "$url"; then ok=1; break; fi
    echo "[fetch-llvm] download attempt $i failed, retrying..."; sleep 4
done
[ "$ok" = "1" ] || { echo "[fetch-llvm] failed to download $url"; exit 1; }

rm -rf "$DEST"
mkdir -p "$DEST"
tar -xf "$tmp/$file" -C "$DEST" --strip-components=1
rm -rf "$tmp"

# The bundle drops the static component archives to stay small, but LLVM/Clang's cmake exports still
# list every one with an existence check that find_package enforces. LoreTLC links only the dylib
# targets (clang-cpp, LLVM) and never these archives, so satisfy the check with empty placeholders
# (bundle-llvm.sh strips them back out of the devkit). Without this, find_package(Clang) aborts with
# "imported target references the file lib<component>.a but this file does not exist".
if [ -d "$DEST/lib/cmake" ]; then
    grep -rhoE 'lib[A-Za-z0-9_.+-]+\.a' "$DEST"/lib/cmake 2>/dev/null | sort -u | while read -r base; do
        f="$DEST/lib/$base"
        [ -e "$f" ] || : > "$f"
    done
fi

echo "[fetch-llvm] $ARCH ready at $DEST ($("$DEST/bin/clang" --version | head -1))"

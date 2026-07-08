#!/usr/bin/env bash
# Build and run the hello and demo thunk examples inside the try container. The devkit is expected at
# $LORELEI_DEVKIT; mount it with: docker run -v /path/to/devkit:/opt/devkit:ro ...
set -euo pipefail

: "${LORELEI_DEVKIT:=/opt/devkit}"
: "${QEMU:=/usr/local/bin/qemu-x86_64}"
: "${PLUGIN:=/opt/libdlcall.so}"

if [ ! -x "$LORELEI_DEVKIT/bin/x86_64-linux-gnu-clang" ]; then
    echo "error: no devkit at $LORELEI_DEVKIT" >&2
    echo "       mount one with: docker run -v /path/to/devkit:/opt/devkit:ro ..." >&2
    exit 1
fi

REPO=/opt/lorelei

run_one() {
    local name=$1
    echo
    echo "==================== $name ===================="
    # Each example builds into its own build/ + install/, so a clean rebuild is self-contained.
    make -s -C "$REPO/examples/$name" \
        LORELEI_DEVKIT="$LORELEI_DEVKIT" QEMU="$QEMU" PLUGIN="$PLUGIN" clean
    make -C "$REPO/examples/$name" \
        LORELEI_DEVKIT="$LORELEI_DEVKIT" QEMU="$QEMU" PLUGIN="$PLUGIN" run
}

run_one hello
run_one demo

echo
echo "==================== done ===================="
echo "Every line above was produced by a native host library, called from an x86_64 guest under qemu."

#!/usr/bin/env bash
#
# Write the x86_64-linux-gnu-clang(++) convenience wrappers into a dist tree. They are for a thunk
# author compiling their own x86_64 guest test programs by hand; the GTL cmake build uses the guest
# toolchain file instead. -fuse-ld=lld so they do not need a system x86_64 binutils.
set -euo pipefail
TREE="$1"

cat > "$TREE/bin/x86_64-linux-gnu-clang" <<'EOF'
#!/bin/sh
tc="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
exec "$tc/bin/clang" --target=x86_64-linux-gnu --sysroot="$tc/x86_64/sysroot" -fuse-ld=lld "$@"
EOF
cat > "$TREE/bin/x86_64-linux-gnu-clang++" <<'EOF'
#!/bin/sh
tc="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
exec "$tc/bin/clang++" --target=x86_64-linux-gnu --sysroot="$tc/x86_64/sysroot" -fuse-ld=lld "$@"
EOF
chmod +x "$TREE/bin/x86_64-linux-gnu-clang" "$TREE/bin/x86_64-linux-gnu-clang++"

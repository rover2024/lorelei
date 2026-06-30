# Changelog

## 1.0.0.0 (2026-06-30)

First release. Lorelei is a cross-ISA compatibility layer that lets an emulated guest call the
host's native libraries directly, so heavy work like graphics, compute and compression runs at
native speed instead of being emulated one instruction at a time.

- Patched QEMU user-mode emulation forced to `guest_base == 0`, plus a `dlcall` TCG plugin that
  intercepts a magic syscall (number 4096) and turns a guest library call into a native call into
  the real host library, with no marshalling and no pointer translation.
- Guest and host runtimes (`LoreGuestRT`, `LoreHostRT`), with coroutine-based nested reentry so
  the host can call back into guest code.
- Thunk Library Compiler (`LoreTLC`), built on Clang LibTooling, generates both sides of a
  library's thunk from its headers, including function-pointer callbacks and variadic functions.
- End-to-end tested on x86_64, aarch64 and riscv64.

Known limitation: global variable thunking is not yet implemented. Functions and callbacks are
supported.

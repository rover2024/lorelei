# Changelog

## 1.0.1.0 (2026-07-01)

Adds prebuilt runtime artifacts, so a target no longer has to build Lorelei from source to run a thunk.

- **Prebuilt runtime trees**, published as release assets: `x86_64-host`, `aarch64-host` and `riscv64-host` (each the host runtime plus the host thunks for that ISA), and `x86_64-guest` (the guest runtime plus the guest thunks). Each is a self-contained, tool-free tree with lorelei and the thunks merged into one flat prefix.
- A tag-triggered `deploy` workflow builds all four on an x86_64 runner: the two x86_64 trees natively, and the aarch64 and riscv64 host trees cross-compiled, with the host thunk source cross-generated for the target ISA. The aarch64 and riscv64 trees are verified end to end on real hardware.
- The Docker install layout is flattened into one shared prefix, and `ffcall` (avcall) is linked statically into the DLCall library, so a deployed tree carries no runtime dependency on `libavcall`.

## 1.0.0.0 (2026-06-30)

First release. Lorelei is a cross-ISA compatibility layer that lets an emulated guest call the host's native libraries directly, so heavy work like graphics, compute and compression runs at native speed instead of being emulated one instruction at a time.

- Patched QEMU user-mode emulation forced to `guest_base == 0`, plus a `dlcall` TCG plugin that intercepts a magic syscall (number 4096) and turns a guest library call into a native call into the real host library, with no marshalling and no pointer translation.
- Guest and host runtimes (`LoreGuestRT`, `LoreHostRT`), with coroutine-based nested reentry so the host can call back into guest code.
- Thunk Library Compiler (`LoreTLC`), built on Clang LibTooling, generates both sides of a library's thunk from its headers, including function-pointer callbacks and variadic functions.
- End-to-end tested on x86_64, aarch64 and riscv64.

Known limitation: global variable thunking is not yet implemented. Functions and callbacks are supported.

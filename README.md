# LORELEI

[![x86_64](https://github.com/rover2024/lorelei/actions/workflows/ci-x86_64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-x86_64.yml)
[![aarch64](https://github.com/rover2024/lorelei/actions/workflows/ci-aarch64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-aarch64.yml)
[![riscv64](https://github.com/rover2024/lorelei/actions/workflows/ci-riscv64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-riscv64.yml)

Lorelei is a cross-ISA compatibility layer that lets an emulated guest program call the host's native libraries directly, so heavy work like graphics and compute runs at native speed instead of being emulated one instruction at a time.

Box64 and FEX pioneered this kind of native-library pass-through, building the thunks into their own dynamic binary translators. Lorelei carries the approach further: it decouples the mechanism from any specific DBT, generates the thunks automatically from a library's headers instead of writing them by hand, and ports it to QEMU.

## How It Works

Lorelei runs the guest under a patched build of QEMU's user-mode emulation, forced to `guest_base == 0` so a guest pointer and a host pointer are the same number. A guest library call becomes a single magic syscall (number 4096) that a QEMU TCG plugin (`dlcall`) intercepts and turns into a native call into the real host library, with no marshalling and no pointer translation. The return value and any host-to-guest callbacks flow back the same way. For the full call path and the runtimes that carry it, see [docs/HowLoreleiWorks.md](docs/HowLoreleiWorks.md).

You do not write the per-library glue by hand. The Thunk Library Compiler (TLC), built on Clang LibTooling, reads a library's headers and generates the guest and host thunks, including the awkward cases of callbacks and variadic functions. See [docs/HowToUseTLC.md](docs/HowToUseTLC.md) for how to use it, and [lorelei-thunks](https://github.com/rover2024/lorelei-thunks) for ready-made thunks (zlib, SDL, ...).

The underlying pass-through mechanism is also demonstrated from scratch in the [QEMU Pass-Through Test](https://github.com/rover2024/qemu-passthrough-test).

## Highlights

- **Native speed for host libraries.** Accelerated libraries such as `zlib` run on the host, not inside the emulator.
- **Zero-copy arguments.** The shared address space means pointers, structs and buffers pass through untouched, with no per-call serialization.
- **Generated glue, not hand-written.** You describe a library and TLC emits both sides of the thunk code for you, including the awkward cases of function-pointer callbacks and variadic functions.
- **Calls in both directions.** The host can call back into guest code (for example a callback the guest registered), and nested reentry is handled with coroutines.
- **Lean runtime.** The guest and host runtime libraries deliberately avoid heavyweight dependencies.

## Build From Source

Lorelei requires `qmsetup` for configuration, so you need to build it first.

```bash
# where lorelei installs, the devkit prefix that lorelei-thunks builds against
export DEVKIT_DIR=/home/user/devkit

git clone --recursive https://github.com/stdware/qmsetup.git
cd qmsetup

cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$DEVKIT_DIR
cmake --build build --target all
cmake --build build --target install
```

Build this project.

```bash
sudo apt install libffcall-dev libllvm-20-dev libclang-20-dev

git clone https://github.com/rover2024/lorelei.git
cd lorelei
```

Lorelei has two kinds of targets that belong to different ISAs: the **host** side (the `LoreHostRT` runtime and the `LoreTLC` tool, built for the host ISA) and the **guest** side (the `LoreGuestRT` runtime, built for the guest ISA, x86_64). Build them in two configures into separate prefixes: the host side into `$DEVKIT_DIR`, the guest runtime into `$DEVKIT_DIR/x86_64`. The two are independent, so build the host side first.

The guest side always uses the x86_64 toolchain (`x86_64-linux-gnu-gcc`, present natively on an x86_64 host and as the cross compiler on an aarch64/riscv64 one), while the host side uses the native compiler.

```bash
# Host side (LoreHostRT + LoreTLC), built with the native host toolchain.
cmake -B build-host -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$DEVKIT_DIR \
    -Dqmsetup_DIR=$DEVKIT_DIR/lib/cmake/qmsetup \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=FALSE
cmake --build build-host --target install

# Guest runtime (LoreGuestRT), built with the x86_64 toolchain.
cmake -B build-guest -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$DEVKIT_DIR/x86_64 \
    -DCMAKE_C_COMPILER=x86_64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-linux-gnu-g++ \
    -Dqmsetup_DIR=$DEVKIT_DIR/lib/cmake/qmsetup \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE
cmake --build build-guest --target install
```

## Dependencies

- llvm (https://github.com/llvm/llvm-project)
- ffcall (https://www.gnu.org/software/libffcall)
- qmsetup (https://github.com/stdware/qmsetup)
- json11 (https://github.com/dropbox/json11)
- printf (https://github.com/mpaland/printf)
- boost (https://www.boost.org): used by the unit tests only

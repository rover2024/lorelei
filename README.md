# LORELEI

[![x86_64](https://github.com/rover2024/lorelei/actions/workflows/ci-x86_64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-x86_64.yml)
[![aarch64](https://github.com/rover2024/lorelei/actions/workflows/ci-aarch64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-aarch64.yml)

Lorelei is a cross-ISA compatibility layer that lets an emulated guest program call the host's native libraries directly, so heavy work like graphics and compute runs at native speed instead of being emulated one instruction at a time.

## How It Works

Make sure that QEMU runs the guest with `guest_base == 0`, so a guest pointer and a host pointer are the same number. There is no marshalling, no copying buffers across a boundary, and no pointer translation.

On top of that, the guest reaches the host through a single magic syscall (number 4096):

1. The guest calls a library function through a generated thunk.
2. The thunk issues syscall 4096 with a small request that describes the call.
3. A QEMU TCG plugin (`dlcall`) intercepts the syscall and hands control to the host runtime.
4. The host runtime `dlopen`/`dlsym`s the real library and invokes the function with the guest's arguments, which are already valid host pointers.
5. The return value (and any callbacks) flow back the same way.

This replaces the RPC-style client/server of the first version with an in-process jump, so a host call costs little more than the syscall itself.

The per-library thunks themselves (zlib, SDL, ...) are not in this repository; they live in [lorelei-thunks](https://github.com/rover2024/lorelei-thunks), which builds against an installed Lorelei and uses its Thunk Library Compiler to generate the guest and host glue for each library.

For a deeper look at how the Thunk Library Compiler generates the thunks and how the guest and host runtimes carry each call across the boundary, see [docs/HowLoreleiWorks.md](docs/HowLoreleiWorks.md).

The underlying pass-through mechanism is also demonstrated from scratch in the [QEMU Pass-Through Test](https://github.com/rover2024/qemu-passthrough-test).

## Highlights

- **Native speed for host libraries.** Accelerated libraries such as `zlib` run on the host, not inside the emulator.
- **Zero-copy arguments.** The shared address space means pointers, structs and buffers pass through untouched, with no per-call serialization.
- **Generated glue, not hand-written.** The Thunk Library Compiler (TLC), built on Clang LibTooling, reads a library's headers and emits the thunk code for you, including the awkward cases of function-pointer callbacks and variadic functions.
- **Calls in both directions.** The host can call back into guest code (for example a callback the guest registered), and nested reentry is handled with coroutines.
- **Lean runtime.** The guest and host runtime libraries deliberately avoid heavyweight dependencies.

## Build From Source

Lorelei requires `qmsetup` for configuration, you need to build it first.

```bash
export INSTALL_DIR=/home/user/install

git clone --recursive https://github.com/stdware/qmsetup.git
cd qmsetup

cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/qmsetup
cmake --build build --target all
cmake --build build --target install
```

Build this project.

```bash
sudo apt install libffcall-dev libllvm-20-dev libclang-20-dev

git clone https://github.com/rover2024/lorelei.git
cd lorelei
```

Lorelei has two kinds of targets that belong to different ISAs: the **host** side (the `LoreHostRT` runtime and the `LoreTLC` tool, built for the host ISA) and the **guest** side (the `LoreGuestRT` runtime, built for the guest ISA, x86_64). 

- `LORE_BUILD_TOOLS` builds the host tools and the TLC.
- `LORE_BUILD_GUEST_TARGETS` selects the guest runtime

The active compiler must match whichever side is enabled. The host runtime builds either way.

### Build on X86_64

The guest and host ISA are the same, so a single x86_64 compiler builds both sides in one configure:

```bash
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE
cmake --build build --target all
cmake --build build --target install
```

### Build on AARCH64/RISC-V64

The host ISA differs from the guest x86_64, so the two sides need two different compilers and two separate builds into separate prefixes: the host side (`LoreHostRT` + `LoreTLC`) into `$INSTALL_DIR`, the guest runtime (`LoreGuestRT`) into `$INSTALL_DIR/x86_64`. The two are independent; build the host side first.

```bash
# Host side (LoreHostRT + LoreTLC), built with the native host toolchain.
cmake -B build-host -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=FALSE
cmake --build build-host --target install

# Guest runtime (LoreGuestRT), built with an x86_64 toolchain.
cmake -B build-guest -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR/x86_64 \
    -DCMAKE_C_COMPILER=x86_64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-linux-gnu-g++ \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE
cmake --build build-guest --target install
```

## Docker One-Click Test

A self-contained Docker image builds everything (the patched QEMU and its `dlcall` plugin, qmsetup, this repository, and the [lorelei-thunks](https://github.com/rover2024/lorelei-thunks) zlib thunk) and runs the full end-to-end test. The host side (this machine's arch) installs under `install/lorelei` and `install/lorethunks`, the x86_64 guest side under `install/x86_64`. On an x86_64 host both come from the native toolchain; on an aarch64 or riscv64 host the guest side is cross-compiled for x86_64 (the bootstrap pulls in the x86_64 cross-toolchain and an x86_64 minizip), so the same image works on all three host architectures.

Build the image from the repository root:

```bash
docker build -f docker/Dockerfile -t lorelei-test .
```

In a PRC network, build with the USTC mirror:

```bash
docker build --build-arg USE_USTC_MIRROR=1 -f docker/Dockerfile -t lorelei-test .
```

Everything is pre-built in the image, so the test run just executes and exits:

```bash
docker run --rm lorelei-test bash docker/scripts/run-tests.sh
```

[`run-tests.sh`](docker/scripts/run-tests.sh) runs the auto tests (`ctest`), the in-tree `ThunkExample` end-to-end test, and a real workload over the zlib thunk: the distribution's unmodified `minizip` binary compressing a generated file, timed three ways (native, emulated under QEMU, and emulated under QEMU with the dlcall plugin). minizip itself stays in the guest; only its zlib calls cross to the host through the thunk, so the lorelei run lands near the native time and well under the fully-emulated one. Override the input size with `-e ARCHIVE_SIZE=128M` if you want a longer run. For an interactive shell instead, run `docker run --rm -it lorelei-test`.

## Dependencies

- llvm (https://github.com/llvm/llvm-project)
- ffcall (https://www.gnu.org/software/libffcall)
- qmsetup (https://github.com/stdware/qmsetup)
- json11 (https://github.com/dropbox/json11)
- printf (https://github.com/mpaland/printf)

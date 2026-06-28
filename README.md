# LORELEI

[![x86_64](https://github.com/rover2024/lorelei/actions/workflows/ci-x86_64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-x86_64.yml)
[![aarch64](https://github.com/rover2024/lorelei/actions/workflows/ci-aarch64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-aarch64.yml)
[![riscv64](https://github.com/rover2024/lorelei/actions/workflows/ci-riscv64.yml/badge.svg)](https://github.com/rover2024/lorelei/actions/workflows/ci-riscv64.yml)

Lorelei is a cross-ISA compatibility layer that lets an emulated guest program call the host's native libraries directly, so heavy work like graphics and compute runs at native speed instead of being emulated one instruction at a time.

## How It Works

Lorelei runs the guest under QEMU's user-mode emulation with `guest_base == 0`, so a guest pointer and a host pointer are the same number. A guest library call becomes a single magic syscall (number 4096) that a QEMU TCG plugin (`dlcall`) intercepts and turns into a native call into the real host library, with no marshalling and no pointer translation. The return value and any host-to-guest callbacks flow back the same way. For the full call path and the runtimes that carry it, see [docs/HowLoreleiWorks.md](docs/HowLoreleiWorks.md).

You do not write the per-library glue by hand. The Thunk Library Compiler (TLC), built on Clang LibTooling, reads a library's headers and generates the guest and host thunks for you, including the awkward cases of callbacks and variadic functions. See [docs/HowToUseTLC.md](docs/HowToUseTLC.md) for how to use it, and [lorelei-thunks](https://github.com/rover2024/lorelei-thunks) for ready-made thunks (zlib, SDL, ...).

The underlying pass-through mechanism is also demonstrated from scratch in the [QEMU Pass-Through Test](https://github.com/rover2024/qemu-passthrough-test).

## Highlights

- **Native speed for host libraries.** Accelerated libraries such as `zlib` run on the host, not inside the emulator.
- **Zero-copy arguments.** The shared address space means pointers, structs and buffers pass through untouched, with no per-call serialization.
- **Generated glue, not hand-written.** The Thunk Library Compiler (TLC), built on Clang LibTooling, reads a library's headers and emits the thunk code for you, including the awkward cases of function-pointer callbacks and variadic functions.
- **Calls in both directions.** The host can call back into guest code (for example a callback the guest registered), and nested reentry is handled with coroutines.
- **Lean runtime.** The guest and host runtime libraries deliberately avoid heavyweight dependencies.

## Docker One-Click Test

The quickest way to see Lorelei end to end. A self-contained Docker image builds everything (the patched QEMU and its `dlcall` plugin, qmsetup, this repository, and the [lorelei-thunks](https://github.com/rover2024/lorelei-thunks) zlib thunk) and runs the full test.

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

[`run-tests.sh`](docker/scripts/run-tests.sh) runs three things:

1. **Auto tests** (`ctest`) for the runtimes and the TLC.
2. **The `ThunkExample` end-to-end test**, the in-tree manual test that drives a generated thunk under QEMU (x86_64 host only).
3. **A real workload over the zlib thunk**: the distribution's unmodified `minizip` binary compressing a generated file, timed three ways (native, emulated under QEMU, and emulated under QEMU with the `dlcall` plugin). minizip itself stays in the guest. Only its zlib calls cross to the host through the thunk, so the lorelei run lands near the native time and well under the fully-emulated one.

<!-- Override the input size with `-e ARCHIVE_SIZE=128M` for a longer run, or get an interactive shell with `docker run --rm -it lorelei-test`. -->

## Build From Source

Lorelei requires `qmsetup` for configuration, you need to build it first.

```bash
# change to your own install location
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

The host ISA differs from the guest x86_64, so the two sides need two different compilers and two separate builds into separate prefixes: the host side (`LoreHostRT` + `LoreTLC`) into `$INSTALL_DIR`, the guest runtime (`LoreGuestRT`) into `$INSTALL_DIR/x86_64`. The two are independent. Build the host side first.

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

## Dependencies

- llvm (https://github.com/llvm/llvm-project)
- ffcall (https://www.gnu.org/software/libffcall)
- qmsetup (https://github.com/stdware/qmsetup)
- json11 (https://github.com/dropbox/json11)
- printf (https://github.com/mpaland/printf)
- boost (https://www.boost.org): used by the unit tests only

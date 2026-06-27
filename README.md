# LORELEI

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

The host ISA differs from the guest x86_64, so the two sides need two different compilers and two separate builds into the same prefix. The host runtime is built either way, so build the **guest** side first and the **host** side last, so the natively-built `LoreHostRT` is the one that ends up installed.

```bash
# Guest runtime (LoreGuestRT), built with an x86_64 toolchain.
cmake -B build-guest -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
    -DCMAKE_C_COMPILER=x86_64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-linux-gnu-g++ \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_TOOLS=FALSE \
    -DLORE_BUILD_GUEST_TARGETS=TRUE
cmake --build build-guest --target install

# Host side (LoreHostRT + LoreTLC), built with the native host toolchain.
cmake -B build-host -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
    -Dqmsetup_DIR=$INSTALL_DIR/qmsetup/lib/cmake/qmsetup \
    -DLORE_BUILD_TOOLS=TRUE \
    -DLORE_BUILD_GUEST_TARGETS=FALSE
cmake --build build-host --target install
```

## Dependencies

- llvm (https://github.com/llvm/llvm-project)
- ffcall (https://www.gnu.org/software/libffcall)
- qmsetup (https://github.com/stdware/qmsetup)
- json11 (https://github.com/dropbox/json11)
- printf (https://github.com/mpaland/printf)

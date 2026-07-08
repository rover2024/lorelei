# hello: a minimal thunk

A one-function host library that an x86_64 guest calls through a thunk, so the real function runs natively on the host:

```c
void hello(const char *name, int lucky);   // prints a one-line greeting
```

The pieces:

- `hello.h` / `hello.c`: the native host library, the thing that really runs.
- `main.c`: the x86_64 guest program, which just calls `hello("world", 7)`.
- `Makefile`: builds the library, generates the thunk with `LoreMakeThunk.py`, and builds the guest program.

## Get The Devkit

You need a lorelei **devkit** (the toolchain, `LoreTLC`, `LoreMakeThunk.py`, runtimes and headers). Grab `lorelei-devkit-<arch>` for your host from a [Lorelei release](https://github.com/rover2024/lorelei/releases) and unpack it:

```bash
# <arch> is your host: x86_64, aarch64 or riscv64. <version> is the release, e.g. 1.0.3.0.
tar -xf lorelei-devkit-<arch>-<version>.tar.xz   # unpacks to lorelei-devkit-<arch>/
```

## Build

Set `DEVKIT` to the unpacked devkit, then `make`. In one go it:

- compiles the host `libhello.so`,
- generates the thunk from it with `LoreMakeThunk.py`,
- compiles the guest `main.elf` against the generated GTL.

```bash
export DEVKIT=/path/to/lorelei-devkit-<arch>
make
```

The thunk-generation step is what this example is really about. On the built library, `make` runs the devkit's `LoreMakeThunk.py`:

```bash
$DEVKIT/bin/LoreMakeThunk.py --name hello --lib build/libhello.so --header hello.h -o install -- -I.
```

It finds its own devkit (from its `bin/` location) and builds both the host thunk (HTL) and the x86_64 guest thunk (GTL) into a self-contained `LORELEI_THUNK_PATH` prefix at `install/`. Flags after `--` go to the thunk compile.

## Run

Running needs the patched `qemu-x86_64` and its `libdlcall.so`. Build them from the [rover2024/qemu](https://github.com/rover2024/qemu) `minimal-passthrough-plugin` branch, or skip qemu entirely and [use the container](#running-in-a-container). Then `make run`:

```bash
export QEMU=/path/to/qemu-x86_64
export PLUGIN=/path/to/libdlcall.so
make run
```

`make run` is just qemu with the dlcall plugin, `LORELEI_THUNK_PATH` pointed at the thunk pack:

```bash
LORELEI_THUNK_PATH=install \
LD_LIBRARY_PATH=$DEVKIT/lib:install/lib:build \
    $QEMU -L $DEVKIT/x86_64/sysroot -plugin $PLUGIN \
    -E LD_LIBRARY_PATH=install/x86_64/lib/x86_64-LoreGTL:$DEVKIT/x86_64/lib \
    build/main.elf
```

`-L` points qemu at the devkit's x86_64 sysroot so it finds the guest's loader and libc. Expected output, printed by the host `libhello.so` from an emulated guest:

```text
Hello, world! Have a wonderful day. Your lucky number is 7.
```

### Running In A Container

If your machine has no qemu or x86_64 rootfs, run this example in a container instead: [../../docker/try-examples](../../docker/try-examples) builds qemu for you and needs only Docker and an unpacked devkit.

For a fuller example with variadic functions and a callback that reenters the guest, see [../demo](../demo).

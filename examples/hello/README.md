# hello: a minimal thunk

A one-function host library that an x86_64 guest calls through a thunk, so the real function runs natively on the host:

```c
void hello(const char *name, int lucky);   // prints a one-line greeting
```

The pieces:

- `hello.h` / `hello.c` — the native host library, the thing that really runs.
- `main.c` — the x86_64 guest program, which just calls `hello("world", 7)`.
- `Makefile` — builds the library, generates the thunk with `LoreMakeThunk.py`, builds the guest, and runs it.

## Generate The Thunk

You need a lorelei **devkit** (the toolchain, `LoreTLC`, `LoreMakeThunk.py`, runtimes and headers). Grab `lorelei-devkit-<arch>` for your host from a [Lorelei release](https://github.com/rover2024/lorelei/releases) and unpack it:

```bash
# <arch> is your host: x86_64, aarch64 or riscv64. <version> is the release, e.g. 1.0.3.0.
tar -xf lorelei-devkit-<arch>-<version>.tar.xz   # unpacks to lorelei-devkit-<arch>/
```

`LoreMakeThunk.py` (in the devkit's `bin/`) turns a library into a thunk in one command. Give it the built `.so`, a name, and the headers that declare the API, plus any compiler flags after `--`. It generates and builds both the host thunk (HTL) and the x86_64 guest thunk (GTL) into a self-contained `LORELEI_THUNK_PATH` prefix:

```bash
DEVKIT=/path/to/lorelei-devkit-<arch>
$DEVKIT/bin/LoreMakeThunk.py --devkit $DEVKIT --name hello \
    --lib build/libhello.so --header hello.h \
    -o install -- -I.
```

The `Makefile` does exactly this, after building `libhello.so` first.

## Build And Run

The `Makefile` runs the whole flow. Running it also needs the patched `qemu-x86_64` and its `libdlcall.so`. Build them from the [rover2024/qemu](https://github.com/rover2024/qemu) `minimal-passthrough-plugin` branch, or skip qemu entirely and [use the container](#running-in-a-container).

First build (only the devkit is needed):

```bash
export LORELEI_DEVKIT=/path/to/lorelei-devkit-<arch>
make
```

Then run the guest under qemu with the dlcall plugin:

```bash
make QEMU=/path/to/qemu-x86_64 PLUGIN=/path/to/libdlcall.so run
```

Expected output, printed by the host `libhello.so` from an emulated guest:

```text
Hello, world! Have a wonderful day. Your lucky number is 7.
```

### Running In A Container

If your machine has no qemu or x86_64 rootfs, run this example in a container instead: [../../docker/try-examples](../../docker/try-examples) builds qemu for you and needs only Docker and an unpacked devkit.

For a fuller example with variadic functions and a callback that reenters the guest, see [../demo](../demo).

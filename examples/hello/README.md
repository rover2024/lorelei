# hello: a thunk as a drop-in library

The same one-function library, `libhello.so`, built two ways: the **guest** build prints `from guest` and the **host** build prints `from host`. An unmodified x86_64 guest program `main` calls `hello("world", 7)`. On its own it runs the guest build under emulation. With the thunk swapped in, the call is forwarded to the host build, running natively. The output tells you which one ran.

The sources are under `src/`:

- `hello.h`: the declaration, shared by both builds and the program.
- `hello_guest.c` / `hello_host.c`: the two implementations, printing `from guest` / `from host`.
- `main.c`: the guest program, which calls `hello("world", 7)`.

`Makefile` builds the two libraries and the program, generates the thunk with `LoreMakeThunk.py`, and runs it both ways.

## Get The Devkit

You need a lorelei **devkit** (the toolchain, `LoreMakeThunk.py`, runtimes and headers). Grab `lorelei-devkit-<arch>` for your host from a [Lorelei release](https://github.com/rover2024/lorelei/releases) and unpack it:

```bash
# <arch> is your host: x86_64, aarch64 or riscv64. <version> is the release, e.g. 1.0.4.0.
tar -xf lorelei-devkit-<arch>-<version>.tar.xz   # unpacks to lorelei-devkit-<arch>/
```

## Build

Set `DEVKIT` to the unpacked devkit, then `make`. It:

- builds the guest `libhello.so` (x86_64) and the guest program `main`, linked against it with an rpath,
- builds the host `libhello.so` (your host's architecture),
- generates the thunk from the host library with `LoreMakeThunk.py`.

```bash
export DEVKIT=/path/to/lorelei-devkit-<arch>
make
```

The thunk-generation step is what this example is about. `make` runs the devkit's `LoreMakeThunk.py` on the host library:

```bash
$DEVKIT/bin/LoreMakeThunk.py --name hello --lib build/host/libhello.so --header hello.h -o thunks -- -Isrc
```

It builds both the host thunk (HTL) and the x86_64 guest thunk (GTL) into a self-contained `LORELEI_THUNK_PATH` prefix at `thunks/`.

## Run

Running needs the patched `qemu-x86_64` and its `libdlcall.so`. Build them from the [rover2024/qemu](https://github.com/rover2024/qemu) `minimal-passthrough-plugin` branch, or skip qemu entirely and [use the container](#running-in-a-container). Then `make run`:

```bash
export QEMU=/path/to/qemu-x86_64
export PLUGIN=/path/to/libdlcall.so
make run
```

`make run` runs the same `main` twice. First on its own, so it loads its own guest library:

```bash
$QEMU -L $DEVKIT/x86_64/sysroot build/guest/main
# hello from guest: world, lucky 7
```

Then under the plugin, with the generated guest `libhello.so` ahead of its own on `LD_LIBRARY_PATH`, so the call reaches the host build:

```bash
LORELEI_THUNK_PATH=thunks \
LD_LIBRARY_PATH=$DEVKIT/lib:build/host \
    $QEMU -L $DEVKIT/x86_64/sysroot -plugin $PLUGIN \
    -E LD_LIBRARY_PATH=thunks/x86_64/lib/x86_64-LoreGTL:$DEVKIT/x86_64/lib \
    build/guest/main
# hello from host: world, lucky 7
```

The guest `LD_LIBRARY_PATH`, passed with `-E`, is searched before the program's rpath, so its generated `libhello.so` replaces the guest build:

- `thunks/x86_64/lib/x86_64-LoreGTL` holds the generated guest `libhello.so`.
- `$DEVKIT/x86_64/lib` holds the guest runtime support shipped with the devkit.

The host `LD_LIBRARY_PATH` is qemu's own search path:

- `$DEVKIT/lib` holds the host runtime support shipped with the devkit.
- `build/host` holds the host `libhello.so` the host thunk dispatches to.

`-L` points qemu at the devkit's x86_64 sysroot so it finds the guest's loader and libc (needed on a non-x86_64 host, harmless on x86_64).

The finished tree:

```text
src/
  hello.h
  hello_guest.c
  hello_host.c
  main.c
build/
  guest/
    libhello.so                            (guest build of the library)
    main                                   (the guest program)
  host/
    libhello.so                            (host build of the library)
thunks/
  lib/x86_64-LoreHTL/libhello_HTL.so       (host thunk)
  x86_64/lib/x86_64-LoreGTL/libhello.so    (guest thunk, the drop-in)
```

### Running In A Container

If your machine has no qemu or x86_64 rootfs, run this example in a container instead: [../../docker/try-examples](../../docker/try-examples) builds qemu for you and needs only Docker and an unpacked devkit.

For a fuller example with variadic functions and a callback that reenters the guest, see [../demo](../demo).

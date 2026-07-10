# hello: a thunk as a drop-in library

The same one-function library, `libhello.so`, built two ways: the **guest** build signs its greeting `(from the guest)` and the **host** build `(from the host)`. An unmodified x86_64 guest program `main` calls `hello("World", 7)`. `main` carries no rpath, so which `libhello.so` it loads is chosen at run time by `LD_LIBRARY_PATH`: point it at the guest build and it runs under emulation, point it at the generated thunk instead and the call is forwarded to the host build, running natively. The output tells you which one ran.

The sources are under `src/`:

- `hello.h`: the declaration, shared by both builds and the program.
- `hello_guest.c` / `hello_host.c`: the two implementations, signing `(from the guest)` / `(from the host)`.
- `main.c`: the guest program, which calls `hello("World", 7)`.

`Makefile` builds the two libraries and the program, generates the thunk with `LoreMakeThunk.py`, and runs it both ways.

## Get The Devkit

You need a lorelei **devkit** (the toolchain, `LoreMakeThunk.py`, runtimes and headers). Grab `lorelei-devkit-<arch>` for your host from a [Lorelei release](https://github.com/rover2024/lorelei/releases) and unpack it:

```bash
# <arch> is your host: x86_64, aarch64 or riscv64. <version> is the release, e.g. 1.0.4.0.
tar -xf lorelei-devkit-<arch>-<version>.tar.xz   # unpacks to lorelei-devkit-<arch>/
```

## Build

Set `DEVKIT` to the unpacked devkit, then `make`. It:

- builds the guest `libhello.so` (x86_64) and the guest program `main` linked against it,
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

It builds both the host thunk (HTL) and the x86_64 guest thunk (GTL) into a self-contained thunk pack at `thunks/`.

## Run

Running needs the patched `qemu-x86_64` and its `libdlcall.so`. Build them from the [rover2024/qemu](https://github.com/rover2024/qemu) `minimal-passthrough-plugin` branch, or skip qemu entirely and [use the container](#running-in-a-container). Then `make run`:

```bash
export QEMU=/path/to/qemu-x86_64
export PLUGIN=/path/to/libdlcall.so
make run
```

`make run` runs the same `main` twice. First with its own guest library on the guest `LD_LIBRARY_PATH`:

```bash
$QEMU -L $DEVKIT/x86_64/sysroot \
    -E LD_LIBRARY_PATH=build/guest \
    build/guest/main
# Hello, World! Your lucky number is 7. (from the guest)
```

Then under the plugin, the call reaches the host build. The generated guest thunk goes on the guest `-E LD_LIBRARY_PATH` in place of its own `libhello.so`, and the host runtime finds the pack from the thunk's own location:

```bash
LD_LIBRARY_PATH=$DEVKIT/lib:build/host \
    $QEMU -L $DEVKIT/x86_64/sysroot -plugin $PLUGIN \
    -E LD_LIBRARY_PATH=$DEVKIT/x86_64/lib:thunks/x86_64 \
    build/guest/main
# Hello, World! Your lucky number is 7. (from the host)
```

The guest `LD_LIBRARY_PATH`, passed with `-E`, is where the emulated program looks for its libraries:

- `$DEVKIT/x86_64/lib` holds the guest runtime support shipped with the devkit.
- `thunks/x86_64` holds the generated guest `libhello.so`, which loads in place of the guest build. It carries the path to its host thunk, so the host runtime loads that directly.

The host `LD_LIBRARY_PATH` is qemu's own search path:

- `$DEVKIT/lib` holds the host runtime support shipped with the devkit.
- `build/host` holds the host `libhello.so` the host thunk dispatches to.

`-L` points qemu at the devkit's x86_64 sysroot so it finds the guest's loader and libc (needed on a non-x86_64 host, harmless on x86_64).

The resulting directory tree:

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
  libhello_HTL.so                          (host thunk)
  x86_64/libhello.so                       (guest thunk, the drop-in)
```

### Running In A Container

If your machine has no qemu or x86_64 rootfs, run this example in a container instead: [../../docker/try-examples](../../docker/try-examples) builds qemu for you and needs only Docker and an unpacked devkit.

For a fuller example with variadic functions and a callback that reenters the guest, see [../demo](../demo).

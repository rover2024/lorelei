# demo: variadic functions and callbacks

A fuller companion to [../hello](../hello). Same drop-in structure (a guest `libdemo.so` that `main` links, a host `libdemo.so` that the thunk dispatches to), built from one source in `src/`. The wrapped library just exercises the harder cases a real thunk hits:

```c
int  demo_add(int lhs, int rhs);                                 // a plain call
void demo_puts(const char *message);                             // a pointer argument
int  demo_printf(const char *format, ...);                       // variadic
int  demo_vprintf(const char *format, va_list args);             // a va_list argument
void demo_qsort(void *base, size_t n, size_t size,               // a callback the host
                int (*compar)(const void *, const void *));      //   calls back into the guest
```

The thunk forwards the variadic / `va_list` arguments to the host, and for `demo_qsort` the host's `qsort` calls the comparator back in the guest mid-call. `main.c` runs each and checks the result.

Unlike hello, demo's output does not reveal whether the guest or host build ran, since both forward the real work to libc. So there is no `from guest` / `from host` marker. `make run` shows the thunked run: the same `main`, with the generated guest thunk swapped in, carrying the calls across to the host.

## Build And Run

Exactly as in [hello](../hello), from this directory:

```bash
export DEVKIT=/path/to/lorelei-devkit-<arch>
make

export QEMU=/path/to/qemu-x86_64
export PLUGIN=/path/to/libdlcall.so
make run
```

`make` builds the guest `libdemo.so` (x86_64) and `main` linked against it, the host `libdemo.so` (your host's architecture), and the thunk from the host library. `make run` runs `main` under the plugin with the guest thunk ahead of its own library on `LD_LIBRARY_PATH`, so the calls reach the host build:

```text
demo_puts: Hello from the guest program
demo_printf: checking 20 + 22 = 42
demo_vprintf: checking 7
demo_qsort: 1 2 3 4 5
```

The sorted numbers came back through a comparator that ran in the guest, called from the host's `qsort` mid-thunk.

### Running In A Container

If your machine has no qemu or x86_64 rootfs, run this example in a container instead: [../../docker/try-examples](../../docker/try-examples) builds qemu for you and needs only Docker and an unpacked devkit.

For the drop-in mechanism spelled out step by step, start with [../hello](../hello).

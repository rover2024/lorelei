# demo: variadic functions and callbacks

A fuller companion to [../hello](../hello). Same one-command build, the wrapped library just exercises the harder cases a real thunk hits:

```c
int  demo_add(int lhs, int rhs);                                 // a plain call
void demo_puts(const char *message);                             // a pointer argument
int  demo_printf(const char *format, ...);                       // variadic
int  demo_vprintf(const char *format, va_list args);             // a va_list argument
void demo_qsort(void *base, size_t n, size_t size,               // a callback the host
                int (*compar)(const void *, const void *));      //   calls back into the guest
```

The thunk forwards the variadic / `va_list` arguments to the host, and for `demo_qsort` the host's `qsort` calls the comparator back in the guest mid-call. `main.c` runs each and checks the result.

## Build And Run

Exactly as in [hello](../hello#build-and-run). Unpack a `lorelei-devkit-<arch>`, then from this directory build (devkit only):

```bash
export LORELEI_DEVKIT=/path/to/lorelei-devkit-<arch>
make
```

Then run the guest under qemu with the dlcall plugin:

```bash
make QEMU=/path/to/qemu-x86_64 PLUGIN=/path/to/libdlcall.so run
```

Expected output, every line from the host `libdemo.so`, the sorted numbers coming back through a guest comparator:

```text
demo_puts: Hello from the guest program
demo_printf: checking 20 + 22 = 42
demo_vprintf: checking 7
demo_qsort: 1 2 3 4 5
```

Every line was produced by the **host** `libdemo.so`, and the sorted numbers came back through a comparator that ran in the guest.

### Running In A Container

If your machine has no qemu or x86_64 rootfs, run this example in a container instead: [../../docker/try-examples](../../docker/try-examples) builds qemu for you and needs only Docker and an unpacked devkit.

For the `LoreMakeThunk.py` command and the minimal case, start with [../hello](../hello).

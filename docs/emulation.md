# Calling Host Libraries From an Emulated Guest: the `hello` Example

This walkthrough takes you from nothing to a working cross-ISA call: an **x86_64 guest program**,
running under QEMU on a **non-x86_64 host** (or on x86_64 itself), calls a function in a **native host
library**. The call leaves the emulated guest, runs at full native speed on the host, and returns. The
example wraps a tiny library called `hello`, but the exact same steps wrap a real one such as zlib.

Everything below is copy-paste runnable on an Ubuntu 24.04 machine.

## How It Works (One Paragraph)

The guest links against a **guest thunk library (GTL)**: a stand-in `libhello.so` whose functions, when
called, issue a single magic syscall instead of doing the work. A small **QEMU plugin** (`libdlcall.so`)
intercepts that syscall and forwards the call to the **host thunk library (HTL)**, which calls the real
native `libhello.so` on the host. Results travel back the same way. The guest never knows it left the
sandbox; the host library never knows it was called from an emulated program.

## What You Need

1. **The patched QEMU with the dlcall plugin.** Build QEMU from the lorelei branch with plugins
   enabled; this produces `qemu-x86_64` and `contrib/plugins/libdlcall.so`. A distribution `qemu-x86_64`
   will **not** work (it lacks the zero-guest-base patch and the plugin).
2. **A lorelei devkit** for your host architecture. Download `lorelei-devkit-<arch>.tar.xz` and unpack
   it. It bundles clang/LLVM, the thunk compiler, the guest cross toolchain and sysroot, so you need no
   system clang. It runs on any host with glibc 2.38 or newer (Ubuntu 24.04+, Debian 12+).
3. **The lorelei-thunks source tree**, which holds the thunk build system and this example under
   `examples/hello/`.
4. **A few system packages.** The devkit's clang links a handful of ubiquitous libraries, and building
   the example needs a compiler and CMake:

   ```bash
   sudo apt-get install -y build-essential cmake ninja-build python3 \
       libedit2 libffi8 libzstd1 libxml2 zlib1g
   ```

Point an environment variable at the unpacked devkit; everything below uses it:

```bash
export LORELEI_DEVKIT=/path/to/lorelei-devkit-x86_64
```

## The Host Library

The library we want the guest to reach. Ordinary C, built as a normal shared object for the host.

`examples/hello/hello.h`

```c
#pragma once

#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*hello_compare_fn)(const void *, const void *);

int hello_add(int lhs, int rhs);
void hello_puts(const char *message);
int hello_printf(const char *format, ...) __attribute__((format(printf, 1, 2)));
int hello_vprintf(const char *format, va_list args);
void hello_qsort(void *base, size_t nmemb, size_t size, hello_compare_fn compar);

#ifdef __cplusplus
}
#endif
```

`examples/hello/hello.c`

```c
#include "hello.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int hello_add(int lhs, int rhs) {
    return lhs + rhs;
}

void hello_puts(const char *message) {
    puts(message);
    fflush(stdout);
}

int hello_printf(const char *format, ...) {
    va_list args;
    int written;

    va_start(args, format);
    written = hello_vprintf(format, args);
    va_end(args);

    return written;
}

int hello_vprintf(const char *format, va_list args) {
    int written = vprintf(format, args);
    fflush(stdout);
    return written;
}

void hello_qsort(void *base, size_t nmemb, size_t size, hello_compare_fn compar) {
    qsort(base, nmemb, size, compar);
}
```

## The Guest Program

An ordinary program that includes `hello.h` and calls its functions. It has no idea a thunk is involved.

`examples/hello/main.c`

```c
#include "hello.h"

#include <stdio.h>

static int compare_ints(const void *lhs, const void *rhs) {
    const int a = *(const int *)lhs;
    const int b = *(const int *)rhs;

    return (a > b) - (a < b);
}

static int hello_vprintf_wrapper(const char *format, ...) {
    va_list args;
    int written;

    va_start(args, format);
    written = hello_vprintf(format, args);
    va_end(args);

    return written;
}

int main(void) {
    int values[] = {5, 1, 4, 2, 3};
    const size_t count = sizeof(values) / sizeof(values[0]);

    hello_puts("hello_puts: Hello from the guest program");
    hello_printf("hello_printf: %s %d + %d = %d\n", "checking", 20, 22,
                 hello_add(20, 22));
    hello_vprintf_wrapper("hello_vprintf: %s %d\n", "checking", 7);

    hello_qsort(values, count, sizeof(values[0]), compare_ints);

    hello_printf("hello_qsort:");
    for (size_t i = 0; i < count; ++i) {
        hello_printf(" %d", values[i]);
    }
    hello_printf("\n");
    return 0;
}
```

## The Thunk Config

One small file tells the thunk compiler which library to wrap and where its header is. It names no
machine paths: the devkit supplies the toolchain, so this file stays portable.

`examples/hello/hello-thunk.toml`

```toml
dir = "hello"
project = "hello"

[symbols]
lib = "examples/hello/build/libhello.so"

[desc]
headers = ["hello.h"]

[manifest]
callback_replace = true
auto_link = true

[cmake]
all_extra_includes = ["${CMAKE_SOURCE_DIR}/examples/hello"]
htl_extra_links = ["${CMAKE_SOURCE_DIR}/examples/hello/build/libhello.so"]
```

## Build and Run

A Makefile in `examples/hello/` ties the four steps together, one rule each (`$(HERE)` is that
directory, `$(REPO)` the lorelei-thunks root):

```make
# 1. the native host library
$(HERE)/build/libhello.so: $(HERE)/hello.c
	cc -shared -fPIC -O2 -o $@ $<

# 2. generate + build + install the thunk (host HTL + guest GTL)
$(INSTALL)/share/lorelei/ThunkDB.json: $(HERE)/build/libhello.so
	cd $(REPO) && python3 scripts/MakeThunk.py --devkit-prefix $(DEVKIT) \
	    --config examples/hello/hello-thunk.toml --force

# 3. the guest program, linked against the GTL
$(HERE)/build/main.elf: $(HERE)/main.c $(INSTALL)/share/lorelei/ThunkDB.json
	$(DEVKIT)/bin/x86_64-linux-gnu-clang $< -I$(HERE) -L$(GTL) -lhello -o $@

# 4. run the guest under qemu + the dlcall plugin
run: all
	LORELEI_ROOT=$(INSTALL) LORELEI_GUEST_ROOT=$(INSTALL)/x86_64 \
	LD_LIBRARY_PATH=$(DEVKIT)/lib:$(INSTALL)/lib:$(HERE)/build \
	  $(QEMU) -L $(DEVKIT)/x86_64/sysroot -plugin $(PLUGIN) \
	  -E LD_LIBRARY_PATH=$(GTL):$(DEVKIT)/x86_64/lib \
	  $(HERE)/build/main.elf
```

The ``-L`` points qemu at the devkit's bundled x86_64 sysroot so it can find the guest's
``ld-linux-x86-64.so.2`` and libc. On a non-x86_64 host that loader is otherwise absent; on x86_64 it
is harmless, since qemu consults the prefix only for paths missing on the host.

Point `LORELEI_DEVKIT` at the unpacked devkit and `QEMU` / `PLUGIN` at the patched build, then run:

```bash
export LORELEI_DEVKIT=/path/to/lorelei-devkit-x86_64
make -C examples/hello \
     QEMU=/path/to/qemu/build/qemu-x86_64 \
     PLUGIN=/path/to/qemu/build/contrib/plugins/libdlcall.so run
```

Expected output:

```text
hello_puts: Hello from the guest program
hello_printf: checking 20 + 22 = 42
hello_vprintf: checking 7
hello_qsort: 1 2 3 4 5
```

Every one of those lines was printed by the **host** `libhello.so`, called from an x86_64 guest running
under emulation.

## What Each Environment Variable Does

The run command sets two search paths, one for each side of the boundary:

- The **host** `LD_LIBRARY_PATH` (before `qemu`) resolves the libraries the plugin loads: the lorelei
  host runtime in `$(DEVKIT)/lib`, the installed HTL in `$(INSTALL)/lib`, and the real host
  `libhello.so` in `$(HERE)/build`. Wrapping a system library (zlib, lzma) needs no third entry,
  because the real library is already on the system.
- The **guest** `LD_LIBRARY_PATH` (after `-E`, which QEMU passes to the guest) resolves the GTL and the
  guest lorelei runtime that the guest program links against.

`LORELEI_ROOT` and `LORELEI_GUEST_ROOT` tell the two runtimes where the thunk database and installed
thunks live.

## Wrapping a Real Library

The `hello` steps are the whole recipe. To wrap, say, zlib, point the config at the system `libz.so`,
list `zlib.h` under `[desc].headers`, drop the custom `[cmake]` entries (a system library needs no extra
include path or link), and skip step 1 (the host library already exists). `MakeThunk.py` scaffolds the
thunk, and the same run command works.

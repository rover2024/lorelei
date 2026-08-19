# Manual End-to-End TLC Test

This drives a real thunk all the way through. From the thunk sources CMake generates for the **ThunkExample** library (the fixture in [`src/tests/auto/TLC/TestData`](../../auto/TLC/TestData)), CMake also builds the guest thunk library, the host thunk library (with the example host implementation) and a guest program. The `run_manual_tlc` target then runs the program under QEMU with the `dlcall` plugin. Every `le_*` call the program makes is carried across to the host implementation.

It is not run by ctest; invoke the target by hand. It only works on an x86_64 host: the guest side reuses the local `LoreGuestRT`, which on an aarch64/riscv64 host is built native and so cannot serve the x86_64 guest. The build targets are skipped entirely on a non-x86_64 host.

## What It Exercises

[`Program.c`](Program.c) is an ordinary x86_64 program that calls the example API and checks the results, covering the same features as the unit test but for real:

- `le_printf` / `le_sscanf` variadic argument marshalling;
- `le_emit` / `le_emit_attr` (printf functions recognised by descriptor / format attribute);
- `le_qsort` / `le_bsearch`, whose comparator the host calls **back into the guest** (reentry);
- `le_mix`, which round-trips a `long double` through the type filter.

## Running

`QEMU_BUILD_DIR` points the test at your QEMU build (it must hold `qemu-x86_64` and `contrib/plugins/libdlcall.so`) and has no default. Give it through the environment when the target runs (no reconfigure), or at configure time with `-DQEMU_BUILD_DIR=...`:

```sh
QEMU_BUILD_DIR=/path/to/qemu/build/release cmake --build <build-dir> --target run_manual_tlc
```

The target builds the guest thunk (`libThunkExample.so`), the host thunk (`libThunkExample_HTL.so`) and the program into the test's build directory, writes a `ThunkDB.json` next to them, and runs. Expected output ends with `ThunkExample guest test: OK`.

## Cross-Architecture Coverage

The CMake target above stays x86_64-only, because one native build tree cannot hold both a native HTL and an x86_64 GTL/program. [`RunMakeThunk.sh`](RunMakeThunk.sh) drives the same fixture through [`LoreMakeThunk.py`](../../../../scripts/LoreMakeThunk.py) instead, which builds each side with the compiler its config names, so it runs on any host architecture. This is what CI runs, on x86_64, aarch64 and riscv64 alike.

It builds the example implementation into a native `libThunkExample.so`, generates the thunk pack from it (passing the fixture's own `Desc.h`, `Symbols.conf` and the two manifests through, since `Manifest_host.cpp` carries the hand-written `long double` type filters), links `Program.c` against the generated GTL, and runs it under qemu.

Point it at a `MakeThunkConfig.json` and a qemu, and give it the compiler and runtime directories that config does not name:

```sh
CONFIG=<prefix>/share/lorelei/MakeThunkConfig.json \
GUEST_CC=x86_64-linux-gnu-gcc \
GUEST_RT=<prefix>/x86_64/lib HOST_RT=<prefix>/lib \
QEMU=/path/to/qemu-x86_64 PLUGIN=/path/to/libdlcall.so \
    ./RunMakeThunk.sh
```

With an unpacked devkit, `DEVKIT=<devkit>` supplies all of `CONFIG`, `GUEST_CC`, `GUEST_RT`, `HOST_RT` and the guest sysroot on its own.

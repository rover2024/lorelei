# Manual End-to-End TLC Test

This drives a real thunk all the way through. From the thunk sources CMake generates for the **ThunkExample** library (the fixture in [`src/tests/auto/TLC/TestData`](../../auto/TLC/TestData)), CMake also builds the guest thunk library, the host thunk library (with the example host implementation) and a guest program. The `run_manual_tlc` target then runs the program under the patched QEMU with the `dlcall` plugin. Every `le_*` call the program makes is carried across to the host implementation.

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

# Changelog

## v1.0.5.0 (2026-07-09)

Runs on stock upstream QEMU, makes a failed library load say why and where from, and simplifies how the examples select the thunk.

- **The shipped devkit now runs on stock upstream QEMU.** Previously the runtime assumed the patched fork's address layout (guest memory below the emulator, host above) to tell guest and host pointers apart, a numeric split at the emulator boundary. It now classifies host addresses with `dladdr` instead, which makes no layout assumption, so the devkit works with an unmodified upstream `qemu-x86_64` and the dlcall plugin. Building with `-DLORE_CONFIG_QEMU_SUPPORT_ADDRESS_SEPARATION=ON` restores the fork's faster numeric classification.
- **Load failures now report the linker error.** Every place the runtime gives up on a `dlopen` or `dlsym` (the guest runtime failing to load the host runtime or resolve its common entry, a guest thunk failing to load its host thunk, the host thunk failing to load or resolve a symbol from the real host library) now appends the `dlerror` string, so a bare `failed to load host runtime` reads `... : <reason>` instead of ending in an unexplained abort. The host-side error is fetched through the plugin, so it is available even during guest-runtime bootstrap.
- **Runtime logs are tagged with their origin.** Each record now carries the emitting runtime as a category (`lorelei.guest-rt` or `lorelei.host-rt`), which the host sink renders as a `name: ` prefix, in place of the hand-written `[GRT]`/`[HRT]`/`[GTL]`/`[HTL]` tags that used to be baked into each message. So the load failure above prints as `lorelei.guest-rt: failed to load host runtime: <reason>`. The dotted category names plug into the existing log filter grammar, so a rule like `lorelei.guest-rt = false` can silence one side.
- **Examples select the thunk purely through `LD_LIBRARY_PATH`.** `examples/hello` and `examples/demo` no longer link `main` with an `$ORIGIN` rpath. The program carries no rpath at all, so which `libX.so` it loads (the guest build, or the generated guest thunk in its place) is decided entirely by the guest `LD_LIBRARY_PATH` passed with `-E`, with nothing implicit in the binary.

## v1.0.4.0 (2026-07-08)

Makes building a thunk a single command. The devkit now ships LoreMakeThunk, a self-contained generator that turns a library and its headers into a ready thunk pack using only the devkit's own clang and LoreTLC, with no build system and nothing to hand-write. Two runnable examples and a container come with it.

- **`LoreMakeThunk`**, a one-command thunk generator installed in the devkit as `bin/LoreMakeThunk.py`. Given a real shared library and the headers that declare its API, it dumps the exported symbols, runs `LoreTLC`, and compiles both the host thunk (HTL) and the x86_64 guest thunk (GTL) into a standard drop-in `LORELEI_THUNK_PATH` prefix. It drives the devkit's own clang directly, with no cmake, make or git, and no manifest or config file to write. Compiler flags follow a `--` separator, clang-tooling style.
- **Boilerplate is generated, or supplied.** The four intermediates LoreMakeThunk normally writes (`Desc.h`, `Symbols.conf`, `Manifest_host.cpp`, `Manifest_guest.cpp`) can each be overridden with `--desc`, `--symbols`, `--manifest-host` and `--manifest-guest`, for the cases a header cannot express on its own, such as a `pass::printf` descriptor for an obscure-named format function or a type filter for `long double`. The function list comes from `--lib` (dumped with nm) or from `--symbols`.
- **Runnable examples, now in this repository.** `examples/hello` (a minimal one-function library) and `examples/demo` (variadic functions and a callback that reenters the guest) each build a thunk and run it under the plugin with `make`. `docker/try-examples` builds qemu and runs both against a mounted devkit, for hosts without qemu or an x86_64 rootfs.

## v1.0.3.0 (2026-07-05)

Decouples thunk distribution from the base. Thunks now ship as standalone drop-in packs discovered by a single search path, so libraries can be added to a target without touching or rebuilding the devkit or runtime, and neither of those grows as more libraries are thunked.

- **Standalone thunk packs**, a new release asset per ISA: `lorelei-thunks-x86_64`, `lorelei-thunks-aarch64` and `lorelei-thunks-riscv64`. Each is a self-contained, drop-in prefix carrying the host thunks (`lib/<arch>-LoreHTL`), the guest thunks (`x86_64/lib/x86_64-LoreGTL`) and a `share/lorelei/ThunkDB.json`. The devkit and runtime trees no longer bundle any thunks.
- **`LORELEI_THUNK_PATH`**, a single colon-separated search path of thunk-pack prefixes, highest priority first, the first to define a thunk winning (like `PATH`). Drop in more packs, or your own build prefix, without rebuilding anything.
- **Per-source `ThunkDB.json`**: each pack's own JSON is layered over that pack's own directory scan (the pack's directories are the JSON's shorthand defaults), processed lowest to highest priority. `LORELEI_THUNK_DATABASE` remains an optional top-level override, and `LORELEI_THUNK_NO_AUTOSCAN` disables scanning.
- **`LORELEI_ROOT` is removed.** It was a redundant single-root base after the split. The lorelei runtime `.so`'s are found on `LD_LIBRARY_PATH` as before.
- The deploy workflow now publishes nine assets (was six): `lorelei-{devkit,runtime,thunks}-<arch>` for x86_64, aarch64 and riscv64, all cut from one cross-built tree, and each tarball is named with the release version.

## v1.0.2.0 (2026-07-05)

Adds a prebuilt devkit, so a thunk can be built for a target without compiling Lorelei or LLVM/Clang from source. It complements the runtime artifacts from 1.0.1.0.

- **Prebuilt devkit trees**, published as release assets: `lorelei-devkit-x86_64`, `lorelei-devkit-aarch64` and `lorelei-devkit-riscv64`. Each is one self-contained prefix bundling the host toolchain (Clang/LLVM, `LoreTLC`, qmsetup), the x86_64 guest cross toolchain and sysroot, and the Lorelei runtimes. A thunk builds against it with only CMake, Ninja and Make on the host.
- The devkit and runtime builds are merged into one cross-built distribution pipeline. A single x86_64 runner produces, per target ISA, a `lorelei-runtime-<arch>` and a `lorelei-devkit-<arch>` tarball, and the deploy workflow runs one job per architecture. The static LLVM/Clang archives are dropped from the devkit, roughly halving its unpacked size.
- **ThunkDatabase auto-discovery**: the host runtime scans the thunk directories for matching guest and host libraries, so `ThunkDB.json` is now optional and only needed to override paths or add aliases.
- Guest callbacks reenter through a shared-entry return-address trampoline on aarch64 and riscv64, replacing the earlier fixed-register machinery.

## 1.0.1.0 (2026-07-01)

Adds prebuilt runtime artifacts, so a target no longer has to build Lorelei from source to run a thunk.

- **Prebuilt runtime trees**, published as release assets: `x86_64-host`, `aarch64-host` and `riscv64-host` (each the host runtime plus the host thunks for that ISA), and `x86_64-guest` (the guest runtime plus the guest thunks). Each is a self-contained, tool-free tree with lorelei and the thunks merged into one flat prefix.
- A tag-triggered `deploy` workflow builds all four on an x86_64 runner: the two x86_64 trees natively, and the aarch64 and riscv64 host trees cross-compiled, with the host thunk source cross-generated for the target ISA. The aarch64 and riscv64 trees are verified end to end on real hardware.
- The Docker install layout is flattened into one shared prefix, and `ffcall` (avcall) is linked statically into the DLCall library, so a deployed tree carries no runtime dependency on `libavcall`.

## 1.0.0.0 (2026-06-30)

First release. Lorelei is a cross-ISA compatibility layer that lets an emulated guest call the host's native libraries directly, so heavy work like graphics, compute and compression runs at native speed instead of being emulated one instruction at a time.

- Patched QEMU user-mode emulation forced to `guest_base == 0`, plus a `dlcall` TCG plugin that intercepts a magic syscall (number 4096) and turns a guest library call into a native call into the real host library, with no marshalling and no pointer translation.
- Guest and host runtimes (`LoreGuestRT`, `LoreHostRT`), with coroutine-based nested reentry so the host can call back into guest code.
- Thunk Library Compiler (`LoreTLC`), built on Clang LibTooling, generates both sides of a library's thunk from its headers, including function-pointer callbacks and variadic functions.
- End-to-end tested on x86_64, aarch64 and riscv64.

Known limitation: global variable thunking is not yet implemented. Functions and callbacks are supported.

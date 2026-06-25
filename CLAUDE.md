# CLAUDE.md — lorelei-v2

Guidance for AI agents working in this repo.

## What this is

**lorelei-v2** is a from-scratch refactor of **lorelei v1** (`../lorelei`). It's a cross-ISA
compatibility layer that lets an emulated guest call native host libraries (OpenGL/Vulkan/SDL/...)
for acceleration.

**Core mechanism (v1 → v2 shift):** v1 used an RPC-style GuestClient/HostServer. v2 uses a **QEMU
TCG plugin** (`../qemu2/contrib/plugins/dlcall.c` → `libdlcall.so`) that intercepts a **magic syscall
(number 4096, `lore::DLCallSyscallNumber`)**. The guest issues it to make the host dlopen/dlsym/invoke
host functions. This requires `guest_base == 0` (shared address space, zero pointer translation) —
hence the **`reserved_va`-patched qemu2** (`../qemu2/build/release/qemu-x86_64`). v1 `Base/PassThrough`
became v2 `DLCall`.

## Architecture map

- `include/lorelei/DLCall/`, `src/libs/DLCall/` — wire protocol (`Protocol.h`), `ThunkDatabase`,
  `ProcDefs.h` (the `StaticThunkContext` cross-side contract), trampolines, `VariadicAdaptor`.
- `src/modules/GuestRT/` — `GuestClient` issues the syscall and drives the reentry loop.
- `src/modules/HostRT/` — `HostServer` exports `LoreCommonHostEntry`, which dispatches
  `DLCallSecondaryID` ops; `Invocation` does coroutine-based nested reentry.
- `src/tools/TLC/` + `src/libs/TLCApi/` — **TLC** (Thunk Library Compiler): `stat`→`generate` (+`dump`)
  Clang-LibTooling pipeline; `DocumentContext`/`ProcSnippet`/`Pass`; Builder(DefaultBuilder,LibCFormat)
  /Guard(TypeFilter,CallbackSubstituter)/Misc(GetProcAddress) passes. Emits GTL/HTL thunk source.
- `include/lorelei/ThunkInterface/` — the **manifest layer** TLC emits into. See below.
- `src/tools/HLR/` — Clang source rewriter for CCG/FDG guards. **OUT OF SCOPE — treat as
  non-existent unless explicitly asked. Do not read or touch it.**

## ThunkInterface (manifest layer)

Public surface a manifest author touches: `Proc.h`, `PassTags.h`, and the entry `Manifest.cpp.inc`.
Everything else lives in `Detail/` (== the `lore::thunk::detail` namespace). The `.cpp.inc` files are
textual fragments `#include`d into a generated TU — not compiled standalone, no include guards,
order/macro-dependent (`LORE_THUNK_HOST`, `LORE_THUNK_BUILD`, the `*_FOREACH` lists).

Manifest authoring contract:
```cpp
// guest:  #include <lorelei/ThunkInterface/Manifest.cpp.inc>
// host:   #define LORE_THUNK_HOST
//         #include <lorelei/ThunkInterface/Manifest.cpp.inc>
```
External manifests live in `../lorelei-thunks/src/*/Manifest_{guest,host}.cpp` (already migrated to
this layout). Detail/ roles: `ManifestPreBuild`/`ManifestOnBuild` (non-build placeholders vs build
impl), `ProcTable` (data+staticThunkContext), `ProcInit` (preInitialize), `ProcImpl` (runtime tail:
isHostAddress + ProcInit + host `LoreExchangeContext`).

See `docs/thunkinterface-dlcall-refactor-2026-06-25.md` for the full structure, assembly chain, and
known weak spots.

## Conventions

- **Type doc comments: LLVM style** — lead with `TypeName - description`, NOT Doxygen `\brief`.
  Members/functions stay plain `///`.
- Keep the `.cpp.inc` extension on manifest fragments (intentional — they're for cpp).
- Internal-only headers/fragments go under a `Detail/` subdir.

## Building / verifying

The whole repo does **not** build standalone — TLC must run to generate thunk TUs, and a full CMake
build needs qmsetup + LLVM/Clang 20. To check changes:
- Single `.cpp`: `g++ -Iinclude -Ibuild/Debug/out/include -Iinclude/lorelei/DLCall ... -std=gnu++20
  -fsyntax-only <file>`.
- Manifest `.inc` assembly: write a fake generated TU (empty `LORE_THUNK_*_FOREACH` macros, optional
  `#define LORE_THUNK_HOST` / `LORE_THUNK_BUILD`) that `#include`s `Manifest.cpp.inc` (+ tail
  `Detail/ProcImpl.cpp.inc` for build mode) and `-fsyntax-only` it for guest+host × PreBuild+OnBuild.

## Status (2026-06)

Repo is mid-refactor; expect occasional stale include paths / typos. The `CProc.h` → `DLCall/ProcDefs.h`
migration of the manifest layer is done. Standing task: fully understand the codebase, then make it
"more proper" / standardized (cleanup, consistency, finishing renames). The one acknowledged weak spot
left in the manifest layer is `Detail/ManifestOnBuild.cpp.inc` (per-specialization `#ifdef` carries two
mirrored bodies; a trait/macro parameterization would dedup it).

# How Lorelei works: TLC and the runtimes

Lorelei lets an emulated guest program call the host's native shared libraries directly. This document explains the two pieces that make that happen:

1. **TLC**, the Thunk Library Compiler, which reads a library's headers and generates the per-library glue (the *thunks*).
2. The **guest and host runtimes**, which carry each individual call across the guest/host boundary at run time.

## Foundation: A Magic Syscall

The guest runs under a patched QEMU with `guest_base == 0`, so a guest pointer and a host pointer are the same number: buffers, structs and opaque handles cross the boundary untouched, with no copying or pointer translation.

On top of that, the guest reaches the host through one reserved syscall, number `4096` (`lore::DLCallSyscallNumber`):

1. The guest issues syscall `4096`, with its first argument selecting the operation and the rest carrying operands.
2. A QEMU TCG plugin (`dlcall`) installs a syscall filter that runs before the call reaches the kernel. When it sees number `4096` it serves the request itself, and the real kernel never sees it.
3. The plugin's own surface is deliberately tiny, just enough to `dlopen` a library, `dlsym` a symbol, and "invoke this host function pointer".
4. Everything richer (marshalling a real call's arguments, host-to-guest callbacks, logging) is built on top of those primitives by the runtimes and the generated thunks, so the plugin stays small and stable.

See [`include/lorelei/DLCall/Protocol.h`](../include/lorelei/DLCall/Protocol.h) for the full wire protocol.

## TLC: Thunk Library Compiler

A *thunk library* is the per-library glue that turns "call `deflate(...)`" into "ask the host to run `deflate`". Writing that by hand for every function of a real library (each with its own argument types, callbacks and variadics) does not scale, so TLC generates it.

TLC is a Clang LibTooling tool: it parses the library's actual headers and emits C++ thunk source for both sides.

### Inputs

A thunk is described by three files (see the [lorelei-thunks](https://github.com/rover2024/lorelei-thunks) repository for worked examples):

- **`Desc.h`** includes the library's headers and declares any per-proc pass descriptors (`ProcFnDesc` / `ProcCbDesc`, for example to route a `printf`-style function through the variadic pass).
- **`Symbols.conf`** lists which functions and callbacks to thunk.
- **`Manifest_guest.cpp` / `Manifest_host.cpp`** are where a human can override a single generated piece by hand (a tricky callback, a function that must be reimplemented locally).

### Commands

The thunk pipeline runs in two stages. Each subcommand takes its inputs, then `--`, then the Clang flags used to parse the headers (the include paths, and for guest generation a `-target` triple).

1. **`stat`**: parse `Desc.h` + `Symbols.conf` into `ThunkStat.json`, a description of every requested proc's real signature. This is architecture independent, so it is produced once and reused (see the lorelei-thunks build notes).

   ```sh
   LoreTLC stat -o ThunkStat.json -c Symbols.conf Desc.h \
       -- -xc++ -I/path/to/lorelei/include -I/path/to/zlib/include
   ```

2. **`generate`**: take `ThunkStat.json` + a manifest and emit `Thunk_guest.cpp` or `Thunk_host.cpp` (selected by `-m`), the source that compiles into the guest thunk library (GTL) or host thunk library (HTL).

   ```sh
   # host side
   LoreTLC generate -o Thunk_host.cpp -s ThunkStat.json -m host Manifest_host.cpp \
       -- -xc++ -I/path/to/lorelei/include -I/path/to/zlib/include

   # guest side (built for the guest ISA)
   LoreTLC generate -o Thunk_guest.cpp -s ThunkStat.json -m guest Manifest_guest.cpp \
       -- -xc++ -target x86_64-pc-linux-gnu -I/path/to/lorelei/include -I/path/to/zlib/include
   ```

Standalone helpers:

1. **`dump`**: given a symbol list (`-c`) and a compilation database (`-p`, the directory holding `compile_commands.json`), it emits a self-contained header that re-declares the requested functions and `#include`s exactly the headers their types come from (synthesizing fallback declarations for types that originate in non-header sources).

   ```sh
   LoreTLC dump -c Symbols.conf -o Consolidated.h -p /path/to/build src/foo.c src/bar.c
   ```

### Generation Pipeline

Internally each proc becomes a `ProcSnippet`, and a series of passes (grouped into the phases `Builder`, `Guard`, `Misc`) write into it:

- **Builder** (`DefaultBuilder`, or `LibCFormat` for variadics) lays down the skeleton and the actual call construction.
- **Guard** (`TypeFilter`, `CallbackSubstituter`) injects type/handle conversions and wraps function-pointer callbacks in trampolines so they remain callable across the boundary.
- **Misc** (`GetProcAddress`) handles special cases such as rewriting a returned host proc address into a guest-callable one.

The emitted code for each proc is split into four layers, the `ProcPhase` chain `Entry -> Adapt -> Caller -> Exec` (defined in [`include/lorelei/DLCall/ProcDefs.h`](../include/lorelei/DLCall/ProcDefs.h)):

| Layer | Role |
|-------|------|
| `Entry` | The wire boundary: converts the raw `args[]` buffer to and from typed arguments. |
| `Adapt` | Typed adaptation injected by the Guard/Misc passes (callback substitution, filters). A plain pass-through by default. A manifest can override just this layer. |
| `Caller` | Constructs the actual call (default forwarding, or the variadic wrapper). |
| `Exec` | The real library call, or the cross-boundary invoke into the other side. |

Keeping the layers separate means a manifest can override one of them (say, hand-write the `Adapt` for an awkward callback) without rewriting the marshalling in `Entry`.

## Runtimes: Carrying Calls Across

The generated thunks sit on top of two small runtime libraries that own the actual syscall and its protocol:

```
  ┌──────────────────────── guest (emulated, guest ISA) ───────────────────────────┐
  │  app calls deflate()  ─►  GTL thunk (Entry: pack args[])  ─►  GuestClient      │
  │                                                              (GuestRT)         │
  └───────────────────────────────────────────────┬────────────────────────────────┘
                                    syscall(4096, DR_InvokeProc, ...)
  ┌───────────────────────────────────────────────┴────────────────────────────────┐
  │  dlcall plugin (inside QEMU)  ─►  LoreCommonHostEntry                          │
  └───────────────────────────────────────────────┬────────────────────────────────┘
                                                  ▼  native call
  ┌──────────────────────── host (native ISA) ─────────────────────────────────────┐
  │  HostServer (HostRT)  ─►  HTL thunk (Entry ► Adapt ► Caller ► Exec)  ─►  real  │
  │                                                                        deflate │
  └────────────────────────────────────────────────────────────────────────────────┘
```

### GuestClient (GuestRT): the client

`lore::mod::GuestClient` runs inside the guest and is the only thing that issues syscall `4096`. Its requests fall into two groups (`lore::DLCallRequestID`):

- The **library-management** primitives (`loadLibrary`, `getProcAddress`, `freeLibrary`, `getLibraryError`, `getHostAttribute`) are served by the `dlcall` plugin directly.
- Everything else (`invokeFunction`, `logMessage`, `getModulePath`, `getThunkInfo`) is sent as a single `DR_InvokeProc` request whose target is the host runtime's common entry. So the guest runtime must install that entry (`setCommonHostEntry`) at startup before any real call is made.

For an actual library call, the GTL `Entry` packs the arguments into an `args[]` array and calls `GuestClient::invokeFunction`, which drives the call to completion (including any reentries, see below).

### HostServer (HostRT): the server

`lore::mod::HostServer` lives in the host process, loaded into QEMU next to the plugin. It exports the `LoreCommonHostEntry` symbol that every `DR_InvokeProc` request lands on. That entry dispatches on a secondary id (`lore::DLCallSecondaryID`): `DS_InvokeFunction` starts a call, `DS_ResumeFunction` resumes one after a reentry, plus `DS_LogMessage`, `DS_GetModulePath` and `DS_GetThunkInfo`.

For a library call, `DS_InvokeFunction` hands control to the HTL thunk, which runs the `Entry -> Adapt -> Caller -> Exec` chain and finally calls the real host `deflate`.

### Both Directions: Callbacks and Reentry

Many libraries call back into code the caller supplied (zlib's `zalloc`/`zfree`, a qsort comparator, an SDL event filter). When the real host function calls such a callback, it must run the guest's function, back across the boundary in the host-to-guest direction.

`HostServer::reenter` handles this: it suspends the in-progress host call using coroutine-based invocation machinery, reenters the guest to run its callback (`DS_ResumeFunction` carries the result back), and then resumes the host call where it left off. Because the machinery is a coroutine stack, callbacks can nest arbitrarily. The `CallbackSubstituter` pass is what arranges, at generation time, for the guest's function pointers to arrive on the host as trampolines that trigger this reentry.

## Putting it All Together: One `deflate` Call

1. The guest app calls `deflate`. It is linked against the GTL (which stands in for `libz`), so it reaches the GTL's `deflate` `Entry`, which packs the arguments into `args[]` and calls `GuestClient::invokeFunction`.
2. `GuestClient` issues `syscall(4096, DR_InvokeProc, ...)` aimed at `LoreCommonHostEntry`.
3. The `dlcall` plugin consumes the syscall and natively calls `LoreCommonHostEntry`, which dispatches `DS_InvokeFunction` into `HostServer`.
4. `HostServer` enters the HTL's `deflate` thunk: `Entry` unpacks `args[]`, `Adapt` swaps the guest `zalloc`/`zfree` pointers for host trampolines, `Caller` forwards, and `Exec` calls the real host `deflate`.
5. When host `deflate` calls `zalloc`, the trampoline triggers `HostServer::reenter`, which suspends the host call and reenters the guest to run the guest's allocator, then resumes.
6. The return value flows back out through the same path to the guest caller.

## See also

- [QEMU Pass-Through Test](https://github.com/rover2024/qemu-passthrough-test): the underlying mechanism, hand-written and built up one idea at a time.
- [`include/lorelei/DLCall/Protocol.h`](../include/lorelei/DLCall/Protocol.h): the wire protocol (request ids, secondary ids, calling conventions).
- [`include/lorelei/DLCall/ProcDefs.h`](../include/lorelei/DLCall/ProcDefs.h): the `ProcPhase` layers and the cross-side `StaticThunkContext`.

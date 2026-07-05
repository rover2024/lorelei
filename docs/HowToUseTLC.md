# How to Use TLC: the Thunk Library Compiler

A *thunk library* is the per-library glue that turns "call `deflate(...)`" into "ask the host to run `deflate`". Writing that by hand for every function of a real library (each with its own argument types, callbacks and variadics) does not scale, so TLC generates it.

TLC is a Clang LibTooling tool: it parses the library's actual headers and emits C++ thunk source for both sides. For the runtime path that glue rides on (the magic syscall and the two runtimes), see [HowLoreleiWorks.md](HowLoreleiWorks.md).

## Inputs

A thunk is described by three files (see the [LoreThunk](https://github.com/rover2024/lorelei-thunks) repository for worked examples):

- **`Desc.h`** includes the library's headers and declares any per-proc pass descriptors (`ProcFnDesc` / `ProcCbDesc`, for example to route a `printf`-style function through the variadic pass).
- **`Symbols.conf`** lists which functions and callbacks to thunk.
- **`Manifest_guest.cpp` / `Manifest_host.cpp`** are where a human can override a single generated piece by hand (a tricky callback, a function that must be reimplemented locally).

## Commands

The thunk pipeline runs in two stages. Each subcommand takes its inputs, then `--`, then the Clang flags used to parse the headers (the include paths, and for guest generation a `-target` triple).

1. **`stat`**: parse `Desc.h` + `Symbols.conf` into `ThunkStat.json`, a description of every requested proc's real signature. This is architecture independent, so it is produced once and reused (see the [LoreThunk](https://github.com/rover2024/lorelei-thunks) build notes).

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

1. **`dump`**: a setup aid, not part of the `stat`->`generate` pipeline. Reach for it when a library has no single clean header to point `Desc.h` at. Given a symbol list (`-c`) and a compilation database (`-p`, the directory holding `compile_commands.json`), it emits a self-contained header that re-declares the requested functions and `#include`s exactly the headers their types come from (synthesizing fallback declarations for types that originate in non-header sources). That generated header can then seed `Desc.h`.

   ```sh
   LoreTLC dump -c Symbols.conf -o Consolidated.h -p /path/to/build src/foo.c src/bar.c
   ```

## Generation Pipeline

Generation is driven by *passes*. A pass is a small code generator: handed one proc and what TLC knows about it (its name, its parameter and return types, any `format` attribute or manifest descriptor), it decides whether it applies and, if so, writes the matching C++ into the proc's body. Each proc starts as an empty `ProcSnippet` (TLC knows only its signature, from `ThunkStat.json`) and is built up by a fixed series of passes, each adding to what the ones before it left.

Each proc is emitted as four layers, the `ProcPhase` chain `Entry -> Adapt -> Caller -> Exec` (defined in [`include/lorelei/DLCall/ProcDefs.h`](../include/lorelei/DLCall/ProcDefs.h)). Every layer's body is laid out as ordered slots (`prolog`, `forward`, `center`, `backward`, `epilog`), so a pass always has a well-defined place to inject into. The passes run in three phases: `Builder` lays the skeleton, `Guard` injects conversions, and `Misc` handles special cases.

The walkthrough follows two functions:
- `qsort`: an ordinary call whose comparator the host has to run back in the guest.
- `printf`: a variadic one whose arguments need re-marshalling.

```c
using compare_fn = int (*)(const void *, const void *);   // qsort's comparator type

void qsort(void *base, size_t n, size_t size, compare_fn cmp);
int  printf(const char *fmt, ...);
```

The bodies below are what TLC generates for them, lightly cleaned up for readability (names shortened, and the comparator type pulled out into a `using` alias).

Each `ProcFn<...>` is keyed by the function, a **direction**, and a layer. The direction is `GuestToHost` for a normal call the guest makes into the host, and `HostToGuest` for a callback the host makes back into the guest (`qsort`'s comparator is one, so it shows up below as `ProcCb<compare_fn, HostToGuest, Entry>`).

**The host side (HTL).** The HTL is the receiver, assembled by the three phases in turn.

**1. Builder lays the skeleton.** `DefaultBuilder` wires the four layers together: `Entry` unpacks the raw `args[]` buffer into typed locals and hands them on, `Caller` forwards to `Exec`, and `Adapt` is, for now, a plain pass-through. `Exec` is never emitted: it is fixed boilerplate that calls the real function. (This is the host, receiving, side. The guest side mirrors it and is shown below.)

```cpp
// Entry: unpack the wire buffer, then hand off to Adapt
void ProcFn<::qsort, GuestToHost, Entry>::invoke(void **args, void *ret, void *metadata) {
    auto &arg1 = *(void **) args[0];
    auto &arg2 = *(size_t *) args[1];
    auto &arg3 = *(size_t *) args[2];
    auto &arg4 = *(compare_fn *) args[3];
    ProcFn<qsort, GuestToHost, Adapt>::invoke(arg1, arg2, arg3, arg4);
}

// Adapt: nothing to adapt yet, forward unchanged
void ProcFn<::qsort, GuestToHost, Adapt>::invoke(void *arg1, size_t arg2, size_t arg3, compare_fn arg4) {
    ProcFn<qsort, GuestToHost, Caller>::invoke(arg1, arg2, arg3, arg4);
}

// Caller: forward to Exec, which calls the real qsort
void ProcFn<::qsort, GuestToHost, Caller>::invoke(void *arg1, size_t arg2, size_t arg3, compare_fn arg4) {
    ProcFn<qsort, GuestToHost, Exec>::invoke(arg1, arg2, arg3, arg4);
}
```

A variadic function takes a different `Builder`. The `...` arguments of a `printf` or `scanf` family member (recognized by name, by a `format` attribute, or by an explicit `pass::printf` descriptor) cannot be forwarded blindly, because the variadic calling convention differs between the guest and host ISAs. For these, `LibCFormat` builds the `Caller` differently. It collects the arguments into a `CVargEntry[]` array, an ISA-independent encoding that tags each variadic argument with its type alongside its value, and hands them to `VariadicAdaptor`, which rebuilds the real call on the host. `Entry` still just unpacks the wire buffer (the variadic pack arrives as a ready-made `CVargEntry[]`, `vargs`), and `Adapt` stays an empty pass-through, because `printf` has no callbacks or type conversions for `Guard` to inject:

```cpp
// Entry: unpack the format string and the variadic pack, hand to Adapt
void ProcFn<::printf, GuestToHost, Entry>::invoke(void **args, void *ret, void *metadata) {
    auto &arg1 = *(const char **) args[0];
    auto &vargs = *(CVargEntry **) args[1];
    auto &ret_ref = *(int *) ret;
    ret_ref = ProcFn<printf, GuestToHost, Adapt>::invoke(arg1, vargs);
}

// Adapt: empty, nothing to convert, forward unchanged
int ProcFn<::printf, GuestToHost, Adapt>::invoke(const char *arg1, CVargEntry *vargs) {
    return ProcFn<printf, GuestToHost, Caller>::invoke(arg1, vargs);
}

// Caller: collect the fixed arguments and let VariadicAdaptor rebuild the real call
int ProcFn<::printf, GuestToHost, Caller>::invoke(const char *arg1, CVargEntry *vargs) {
    int ret;
    CVargEntry argv1[] = { CVargGet(arg1) };   // the fixed arguments (just the format string here)
    CVargEntry vret;
    vret.type = CVargTypeID(ret);
    VariadicAdaptor::call(ProcFn<printf, GuestToHost, Exec>::get(),
                          sizeof(argv1) / sizeof(argv1[0]), argv1, -1, vargs, &vret);
    ret = CVargValue(int, vret);
    return ret;
}
```

The `va_list` form (`vprintf`) is identical, except the `Caller` uses `VariadicAdaptor::vcall`.

**2. Guard fills `Adapt` with conversions.** `arg4` is a guest function pointer the host cannot call directly, so the `CallbackSubstituter` pass rewrites `Adapt` alone: it drops trampoline setup into the `forward` slot (run before the call) and teardown into the `backward` slot (run after), leaving `Entry` and `Caller` exactly as Builder left them.

```cpp
void ProcFn<::qsort, GuestToHost, Adapt>::invoke(void *arg1, size_t arg2, size_t arg3, compare_fn arg4) {
    // forward: wrap the guest comparator in a trampoline the host can call
    qsort_LocalContext ctx;   // one CallbackContext per callback arg, held for the call's duration
    ctx.arg4.init<true>((void *&) arg4,
        allocCallbackTrampoline<ProcCb<compare_fn, HostToGuest, Entry>::invoke>);
    // center: the real call now sees a host-callable arg4
    ProcFn<qsort, GuestToHost, Caller>::invoke(arg1, arg2, arg3, arg4);
    // backward: restore the original pointer
    ctx.arg4.fini();
}
```

The other `Guard` pass, `TypeFilter`, injects value conversions into the same slots. For a `long double` argument it drops a `ProcArgFilter<long double>::filter(...)` call into `forward` and a matching `ProcReturnFilter<...>` into `backward`, each calling the conversion the manifest registered for that type.

**3. Misc handles special cases.** A function that returns a host function pointer (a `dlsym` or `*GetProcAddress`-style API) needs that returned address turned into a guest-callable one, which the `GetProcAddress` pass injects. Most procs need nothing from this phase.

**The guest side (GTL).** The GTL is the sender, the same procs compiled a second time into a mirror image (`generate -m guest`). It exports the real `qsort` symbol as a plain alias for the typed `Entry`, so the guest program's own `qsort(...)` call enters the chain directly:

```cpp
// the exported real symbol is an alias for Entry::invoke
LORE_DECL_EXPORT void qsort(void *base, size_t n, size_t size, compare_fn cmp)
    __attribute__((alias("<mangled ProcFn<qsort, GuestToHost, Entry>::invoke>")));
```

`Entry` and `Adapt` carry the typed arguments straight down. `Caller` is where the wire buffer is built, taking the address of each argument into an `args[]` array, and `Exec` (fixed boilerplate again) hands that array to `GuestClient::invokeFunction`, which issues the syscall:

```cpp
// Caller (guest side): pack the typed arguments into the args[] wire buffer
void ProcFn<::qsort, GuestToHost, Caller>::invoke(void *arg1, size_t arg2, size_t arg3, compare_fn arg4) {
    void *args[] = { (void *) &arg1, (void *) &arg2, (void *) &arg3, (void *) &arg4 };
    ProcFn<qsort, GuestToHost, Exec>::invoke(args, nullptr, nullptr);   // Exec issues the syscall
}
```

That `args[]` is exactly what the host `Entry` shown earlier unpacks.

The variadic `printf` mirrors its host `Caller` instead. On the guest, `Entry` is the exported `int printf(const char *fmt, ...)`, and it uses the format string to extract the `...` pack into the same `CVargEntry[]` wire form the host rebuilds the call from:

```cpp
// Entry (guest side): the exported variadic symbol, extracts ... into a CVargEntry[] pack
int ProcFn<::printf, GuestToHost, Entry>::invoke(const char *arg1, ...) {
    int ret;
    CVargEntry vargs[LORE_THUNK_VARG_MAX];
    va_list ap;
    va_start(ap, arg1);
    VariadicAdaptor::extract(VariadicAdaptor::PrintF, arg1, ap, vargs);   // the format string drives the extraction
    va_end(ap);
    ret = ProcFn<printf, GuestToHost, Adapt>::invoke(arg1, vargs);
    return ret;
}
```

`Adapt` and `Caller` then carry `(arg1, vargs)` on to `Exec` as before. So a variadic pack is extracted once on the guest (`extract`, driven by the format string) and rebuilt once on the host (`VariadicAdaptor::call`).

The four layers that come out stay separate, so a manifest can override exactly one of them (hand-write the `Adapt` for an awkward callback, say) without disturbing the marshalling Builder put in `Entry`:

| Layer | Role |
|-------|------|
| `Entry` | The wire boundary. On the host it unpacks the `args[]` buffer into typed arguments. In the GTL it is the exported real symbol (`qsort`) that the guest program calls. |
| `Adapt` | Typed adaptation injected by the `Guard` / `Misc` passes (callback substitution, filters). A plain pass-through by default, overridable from a manifest. |
| `Caller` | Constructs the call: on the host it forwards to `Exec` or wraps a variadic call, and on the guest it packs the arguments into the `args[]` buffer. |
| `Exec` | Fixed boilerplate, not emitted per proc: the real library call on the host, the syscall that crosses the boundary on the guest. |

## Descriptors and Auto-Detection

Most procs need no descriptor at all: `generate` picks the right `Builder` from the function's signature, its name, or its GNU `format` attribute. A descriptor in `Desc.h` is only for the cases none of those cover.

Take SDL's two logging calls, a `...` form and its `va_list` sibling:

```c
// the ... form carries a printf format attribute (SDL spells it SDL_PRINTF_VARARG_FUNC(3))
void SDL_LogMessage (int category, SDL_LogPriority priority, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));

// the va_list form carries none
void SDL_LogMessageV(int category, SDL_LogPriority priority, const char *fmt, va_list ap);
```

Both are printf-style, so their `Caller` has to re-marshal the arguments rather than forward the call blindly (the `va_list` form going through `VariadicAdaptor::vcall`). TLC decides that on its own, by two rules:

- **Name.** A function whose name ends in `printf` or `scanf` is routed to the matching pass: `printf` / `scanf` for the `...` form, `vprintf` / `vscanf` for the `va_list` form. Neither `SDL_LogMessage` nor `SDL_LogMessageV` ends in either, so this rule fires for neither.
- **GNU `format` attribute.** A function carrying `__attribute__((format(printf, fmtIdx, firstVararg)))` is recognized even when its name reveals nothing. SDL tags the `...` form `SDL_LogMessage` with `format(printf, 3, 4)`: the `3` marks the format string as the third parameter and the `4` its first variadic argument, so TLC picks the plain `printf` pass for it automatically, with no descriptor.

`SDL_LogMessageV` is the case that escapes both rules: its name ends in neither `vprintf` nor `vscanf`, and SDL puts no `format` attribute on the `va_list` form (a `va_list` has no `...` arguments for such an attribute to type-check). With nothing for TLC to detect, spell it out in `Desc.h`:

```cpp
template <>
struct ProcFnDesc<::SDL_LogMessageV> {
    // format string at arg 3, va_list at arg 4
    _DESC pass::vprintf<3, 4> builder_pass = {};
};
```

The builder tags are `pass::printf`, `pass::vprintf`, `pass::scanf` and `pass::vscanf`, each taking `<FormatIndex, VariadicIndex>` (1-based, the variadic index being the `...` or `va_list` position). Callbacks are described the same way with `ProcCbDesc`. The tags live in [`include/lorelei/ThunkInterface/PassTags.h`](../include/lorelei/ThunkInterface/PassTags.h).

## See Also

- [`include/lorelei/DLCall/ProcDefs.h`](../include/lorelei/DLCall/ProcDefs.h): the `ProcPhase` layers and the cross-side `StaticThunkContext`.
- [LoreThunk](https://github.com/rover2024/lorelei-thunks): ready-made thunks (zlib, SDL, ...) and the worked examples the inputs above are drawn from.
- [HowLoreleiWorks.md](HowLoreleiWorks.md): the runtime call path the generated thunks run on.

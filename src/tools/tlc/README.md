# Thunk Library Compiler (TLC)

A tool that generates guest and host thunk library sources.

## Usage

```bash
loretlc <input> \
    [--output-header <path>] \
    [--output-source <path>] \
    [-- <compile commands>]
```

- config: a text file in which each line should be a function name

## Concepts

### Runtime
- GRT: Guest Runtime
- HRT: Host Runtime

### Thunk Library
- GTL: Guest Thunk library
- HTL: Host Thunk library

### Thunk Proc
- GTP: Guest Thunk Proc of a library API
- HTP: Host Thunk Proc of a library API
- GCB: Guest Callback thunk proc
- HCB: Host Callback thunk proc

### Control Flow

#### Library API Call
Guest program may execute a function call to the host library, the control flow transfers as follows.

Take `SDL_malloc` as an example:
1. Guest program
2. GTP `__GTP_SDL_malloc` in GTL `libSDL2.so`, here
    - Variadic arguments will be packed
3. GTP wrapper `___GTP_SDL_malloc` in GTL `libSDL2.so`, here
    - Call `Lore_InvokeHostProc` in GRT
5. GRT API `Lore_InvokeHostProc` in GRT `libloregrt.so`
6. Magic syscall handler in emulator
7. HTP `__HTP_SDL_malloc` in HTL `libSDL2_HTL.so`, here
    - Guest callbacks will be replaced as a corresponding host repeater
8. HTP unwrapper `___HTP_SDL_malloc` in HTL `libSDL_HTL.so`, here
    - Variadic arguments will be unpacked
9. Host API `SDL_malloc` in host library `libSDL2.so`

#### Callback
Host library may need to execute callback in guest program, which is replaced as a repeater in HTP, the control flow transfers as follows.

Take `SDL_qsort` as an example (the callback is `compare`):
1. Host API `SDL_qsort` in host library `libSDL2.so`
2. Host repeater in HTL `libSDL2_HTL.so`, here
    - `Lore_HRTThreadCallback` will be set to the original guest callback which has been stored in `__HTP_SDL_malloc`
3. GCB HTP `__HTP_GCB_compare` in `libSDL2_HTL.so`, here
    - Variadic arguments will be packed
4. GCB HTP wrapper `___HTP_GCB_compare` in `libSDL2_HTL.so`, here
    - Call `Lore_ExecuteCallback` in emulator
5. `Lore_ExecuteCallback` in emulator
6. GCB GTP `__GTP_GCB_compare` in `libSDL2.so`, here
    - Host callbacks will be replaced as a corresponding guest repeater
7. GCB GTP unwrapper `___GTP_GCB_compare` in `libSDL2.so`, here
    - Variadic arguments will be unpacked
8. `compare` in guest program

### TLC Pass

API Thunks and Callback Thunks may have to be p

## Input File

```c
// Include the headers containing declaration of the functions
// e.g. SDL2
#include <SDL2/SDL.h>

// Include lorelei files
#include <lorelei/loreshared.h>
#include <lorelei/loreuser.h>

// Include pre-definitions
#include <lorelib_common/manifest-predef.h>


// Option macros
// 1. the most pointer dereferences when replacing
// function pointers in arguments
#define LORELIB_GCB_AUTO_DEPTH 1
#define LORELIB_HCB_AUTO_DEPTH 1


// Generated code


```
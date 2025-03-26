# CFI Compiler (CFIC)

A tool to add CFI checks to C source files.

## Usage

```bash
lorecfic [-c <config>] [-p <compile_commands>] [<input>] [-- <cc options>]
```

- config: a json file with desired function signatures
    ```json
    {
        "functions": [
            "void (int, int)",
            "void *()"
        ]
    }
    ```

- modes:
    - single file mode:
        ```bash
        lorecfic main.cpp -c config.json -- -I/usr/include -DDEBUG
        ```
    - batch mode:
        ```bash
        lorecfic -p /path/to/compile_commands.json -c config.json
        ```

## Working Procedure

Briefly, for each C source file, the tool will:
1. Parse the file and collect all function pointer calls.
2. Check if the function pointer call matches the desired function signatures in the config file.
3. If a match is found, add the necessary CFI checks.
4. Override the original source file with the modified contents.

NOTICE: There should be no unexpanded macros in the source code file

## Implementation Details

### How to Add CFI Checks

Example 1:
```c
typedef int (*FUNC_PTR) (int, int);

void some_method(FUNC_PTR fp) {
    // ...
    a = fp(x, y);
    // ...
}
```

The CFI check will be added as follows:
```c
/****************************************************************************
** Lifted code from reading C++ file 'xxx.c'
**
** Created by: Lorelei CFI compiler
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

//
// CFI declarations begin
//

// Generates the helpers
static void (*LoreLib_ECB)(void *, void *, void *[], void *); // guest entry
static __thread void *LoreLib_Callback;
#define LORELIB_CFI_BASE(FP, CFI_FP)                                \
    ({                                                              \
        __auto_type _ret = (FP);                                    \
        if ((unsigned long) _ret < (unsigned long) LoreLib_ECB) {   \
            LoreLib_Callback = _ret;                                \
            _ret = (__typeof__(_ret)) CFI_FP;                       \
        }                                                           \
        _ret;                                                       \
    })

// Generates the proceding codes for each kind of function pointer
static const void *const LoreLib_CFI_iXii_ptr;
#define LORELIB_CFI_iXii(FP) \
    LORELIB_CFI_BASE(FP, LoreLib_CFI_iXii_ptr)

//
// CFI declarations end
//



//
// Original code begin
//
typedef int (*FUNC_PTR) (int, int);

void some_method(FUNC_PTR fp) {
    // ...

    // Replace the function pointer with the CFI check
    a = LORELIB_CFI_iXii(fp)(x, y);
    // ...
}
//
// Original code end
//



//
// CFI implementation begin
//

// Generates common function declarations
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// lorelei
extern bool Lore_RevealLibraryPath(char *buffer, const void *addr);
extern void *Lore_GetFPExecuteCallback();
extern void *Lore_GetCallbackThunk(const char *sign);


// Generates thunks
static void *LoreLib_GTK_iXii;

// Generates utilities
static inline void LoreLib_GetThunkHelper(
    void **thunk, const char *sign, const char *path) {
    if (!(*thunk = Lore_GetCallbackThunk(sign))) {
        fprintf(stderr, "%s: guest thunk lookup error: %s\n", path, sign);
        abort();
    }
}
static void __attribute__((constructor)) LoreLib_Init() {
    char path[PATH_MAX];
    if (!Lore_RevealLibraryPath(path, LoreLib_Init)) {
        fprintf(stderr, "Unknown host library: failed to get library path\n");
        abort();
    }
    LoreLib_ECB = (__typeof__(LoreLib_ECB)) Lore_GetFPExecuteCallback();
    
    // Initialize thunks
    LoreLib_GetThunkHelper(&LoreLib_GTK_iXii, "int (int, int)", path);
}

// Generates CFI functions
static int LoreLib_CFI_iXii(int arg1, int arg2) {
    int ret;
    void *args[] = { &arg1, &arg2 };
    LoreLib_ECB(LoreLib_GTK_iXii, LoreLib_Callback, args, &ret);
    return ret;
}
static const void *const LoreLib_CFI_iXii_ptr = LoreLib_CFI_iXii;


// For the CFI function:
// 1. the name is associated with the function pointer signature,
//    for different types of function pointers, their names must be
//    different, can take SHA256 or other encoding
// 2. the signature should be the same as the function pointer

// An alternative CFI check function could also be like this:
// static int LoreLib_CFI_iXii(__typeof__(int) arg1, __typeof__(int) arg2)

// Complex types such as C array or function pointer can be declared as common
// types using GNU extension `__typeof__`

//
// CFI implementation end
//
```

Example 2:
```c
typedef int (*FUNC_PTR) (int, int);

typedef FUNC_PTR (*RETURN_FUNC_PTR) (int, int);

void some_method(RETURN_FUNC_PTR fp) {
    // ...
    a = fp(x, y)(z, w);
    // ...
}
```

The CFI check will be added as follows:
```c
// ...

typedef int (*FUNC_PTR) (int, int);

typedef FUNC_PTR (*RETURN_FUNC_PTR) (int, int);

// Generated CFI check function declarations
int check_iXii(FUNC_PTR _callback, int _arg1, int _arg2);
FUNC_PTR check_pXii(RETURN_FUNC_PTR _callback, int _arg1, int _arg2);

void some_method(RETURN_FUNC_PTR fp) {
    // ...

    // A function pointer that returns a function pointer
    // needs to be replaced more than once
    a = LORELIB_CFI_iXii(LORELIB_CFI_iXii(fp)(x, y))(z, w);
    // ...
}

// ...
```

## Dependencis

- LLVM libtooling
- json11
# CFI Compiler (CFIC)

A tool to add CFI checks to C source files.

## Usage

```bash
lorecfic [-i <id>] [-c <config>] [-o <output>] [<input>] -- [compile commands]
```
- id: library identifier
- config: a text file in which each line should be a function signature
    ```
    void (int, int)
    void *()
    ```
- output: output file , default to `stdout` if not specified

Example:
```bash
lorecfic -c config.txt -i mylib mylib.cpp -- -I/usr/include -DDEBUG
```

## Working Procedure

Briefly, for each C source file, the tool will:
1. Parse the file and collect all function pointer calls.
2. Check if the function pointer call matches the desired function signatures in the config file.
3. If a match is found, add the necessary CFI checks.

NOTICE: There should be no unexpanded macros in the input source file

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
** CFI wrapped code from reading C file 'cfi_example_before.c'
**
** Created by: Lorelei CFI compiler
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

//
// CFI declarations begin
//
enum LoreLib_Constants {
    LoreLib_CFI_Count = 1,
};

extern __thread void *Lore_HRTThreadCallback;

typedef void (*FP_NotifyHostLibraryOpen)(const char * /*identifier*/);

static void *LoreLib_AddressBoundary;
static FP_NotifyHostLibraryOpen LoreLib_NHLO;
static const char LoreLib_Identifier[] = "mylib";
static void *LoreLib_CFIs[LoreLib_CFI_Count];

#define LORELIB_CFI(INDEX, FP)                                                             \
    ({                                                                                     \
        __auto_type _lorelib_cfi_ret = (FP);                                               \
        if ((unsigned long) _lorelib_cfi_ret < (unsigned long) LoreLib_AddressBoundary) {  \
            Lore_HRTThreadCallback = _lorelib_cfi_ret;                                     \
            _lorelib_cfi_ret = (__typeof__(_lorelib_cfi_ret)) LoreLib_CFIs[INDEX];         \
        }                                                                                  \
        _lorelib_cfi_ret;                                                                  \
    })

// decl: int (*)(int, int)
#define LORELIB_CFI_1(FP) LORELIB_CFI(1, FP)
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
    a = LORELIB_CFI_1(fp)(x, y);
    // ...
}
//
// Original code end
//



//
// CFI implementation begin
//
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern bool Lore_RevealLibraryPath(char *buffer, const void *addr, bool followSymlink);
extern void *Lore_HrtGetLibraryThunks(const char *path, bool isGuest);
extern struct LoreEmuApis *Lore_HrtGetEmuApis();

static void __attribute__((constructor)) LoreLib_Init() {
    char path[PATH_MAX];
    if (!Lore_RevealLibraryPath(path, LoreLib_Init, false)) {
        fprintf(stderr, "Unknown host library: failed to get library path\n");
        abort();
    }

    struct _LoreEmuApis {
        void *apis[5];
    };
    struct _LoreEmuApis *emuApis = (struct _LoreEmuApis *) Lore_HrtGetEmuApis();
    LoreLib_AddressBoundary = emuApis->apis[0];
    LoreLib_NHLO = emuApis->apis[4];

    void **thunks = (void **) Lore_HrtGetLibraryThunks(LoreLib_Identifier, false);
    if (!thunks) {
        LoreLib_NHLO(LoreLib_Identifier);
        thunks = (void **) Lore_HrtGetLibraryThunks(LoreLib_Identifier, false);
        if (!thunks) {
            fprintf(stderr, "%s: failed to get host library thunks\n", path);
            abort();
        }
    }
    for (int i = 0; i < LoreLib_CFI_Count; ++i) {
        LoreLib_CFIs[i] = thunks[i];
    }
}

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
// decl: int (*)(int, int)
#define LORELIB_CFI_1(FP) LORELIB_CFI(1, FP)
// decl: int (*(*)(int, int))(int, int)
#define LORELIB_CFI_2(FP) LORELIB_CFI(2, FP)

//
// Original code end
//
typedef int (*FUNC_PTR) (int, int);

typedef FUNC_PTR (*RETURN_FUNC_PTR) (int, int);

void some_method(RETURN_FUNC_PTR fp) {
    // ...

    // A function pointer that returns a function pointer
    // needs to be replaced more than once
    a = LORELIB_CFI_2(LORELIB_CFI_1(fp)(x, y))(z, w);
    // ...
}
//
// Original code end
//
```

## Dependencies

- LLVM libtooling
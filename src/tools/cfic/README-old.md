# CFI Compiler

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
typedef int (*FUNC_PTR) (int, int);

// Generates the CFI check function declaration above the function where the
// call expression resides
int check_iXii(FUNC_PTR _callback, int _arg1, int _arg2);

// For the CFI check function:
// 1. the name is associated with the function pointer signature,
//    for different types of function pointers, their names must be
//    different, can take SHA256 or other encoding
// 2. the return type should be the same as the function pointer
// 3. the first argument should be the function pointer type
// 4. the remaining arguments should be the arguments of the function pointer

// An alternative CFI check function could also be like this:
// int check_iXii(__typeof__(int (*)(int, int)) _callback, int _arg1, int _arg2);

// Complex types such as C array or function pointer can be declared as common
// types using GNU extension `__typeof__`

void some_method(FUNC_PTR fp) {
    // ...

    // Replace the function pointer call with the CFI check
    a = check_iXii(fp, x, y);
    // ...
}
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
typedef int (*FUNC_PTR) (int, int);

typedef FUNC_PTR (*RETURN_FUNC_PTR) (int, int);

// Generated CFI check function declarations
int check_iXii(FUNC_PTR _callback, int _arg1, int _arg2);
FUNC_PTR check_pXii(RETURN_FUNC_PTR _callback, int _arg1, int _arg2);

void some_method(RETURN_FUNC_PTR fp) {
    // ...

    // A function pointer that returns a function pointer
    // needs to be replaced more than once
    a = check_iXii(check_pXii(fp, x, y), z, w);
    // ...
}
```

Example 3:
```c
struct some_struct {
    int (*func_ptr)(int);
};

#define CALL_FUNC(S, X) S->func_ptr(X)

void some_method(struct some_struct *s, ...) {
    // ...
    a = CALL_FUNC(s, x);
    // ...
}
```

The CFI check will be added as follows:
```c
struct some_struct {
    int (*func_ptr)(int);
};

#define CALL_FUNC(S, X) S->func_ptr(X)

// Generated CFI check function declarations
int check_iXi(__typeof__(int (*)(int)) _callback, int _arg1);

void some_method(struct some_struct *s, ...) {
    // ...

    // If the function pointer call is in a macro, need to
    // expand the macro completely before replacing it
    a = check_iXi(s->func_ptr, x);
    // ...
}
```

## Dependencis

- openssl
- LLVM libtooling
- json11
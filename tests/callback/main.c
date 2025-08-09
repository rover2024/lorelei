#include <stdio.h>

#include <lorelei/loreshared.h>

#include <lorelib_common/callback.h>

typedef int (*FOO)(int a, int b);

static __attribute__((used)) int foo_thunk(int a, int b) {
    LORELIB_GET_LAST_GCB(callback)
    printf("Saved callback: %p\n", callback);
    printf("foo(%d, %d) = %d\n", a, b, ((FOO) callback)(a, b));
    return a + b;
}

LORELIB_GCB_THUNK_ALLOCATOR(foo_thunk_alloc, foo_thunk)

static int foo(int a, int b) {
    return a + b;
}

int main(int argc, char *argv[]) {
    int a = argc;
    int b = argc * 2;
    __auto_type new_foo = foo_thunk_alloc(foo);
    printf("foo(%d, %d) = %d\n", a, b, new_foo(a, b));
    return 0;
}
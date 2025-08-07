#include <stdio.h>

#include <lorelei/loreshared.h>

#include <lorelib_common/callback.h>

__thread void *Lore_HRTThreadCallback;

LORELIB_GCB_THUNK_ASM_IMPL(foo)
LORELIB_GCB_THUNK_ASM_ALLOCATOR(foo)

typedef int (*FOO)(int a, int b);

static __attribute__((used)) int __HTP_GCB_foo(int a, int b) {
    printf("Saved callback: %p\n", Lore_HRTThreadCallback);
    printf("foo(%d, %d) = %d\n", a, b, ((FOO) Lore_HRTThreadCallback)(a, b));
    return a + b;
}

static int foo(int a, int b) {
    return a + b;
}

int main(int argc, char *argv[]) {
    int a = argc;
    int b = argc * 2;
    __auto_type new_foo = (FOO) foo_GCB_THUNK_alloc(foo);
    printf("foo(%d, %d) = %d\n", a, b, new_foo(a, b));
    return 0;
}
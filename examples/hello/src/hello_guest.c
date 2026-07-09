#include "hello.h"
#include <stdio.h>

void hello(const char *name, int lucky)
{
    printf("hello from guest: %s, lucky %d\n", name, lucky);
    fflush(stdout);
}

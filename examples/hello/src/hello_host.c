#include "hello.h"
#include <stdio.h>

void hello(const char *name, int lucky)
{
    printf("hello from host: %s, lucky %d\n", name, lucky);
    fflush(stdout);
}

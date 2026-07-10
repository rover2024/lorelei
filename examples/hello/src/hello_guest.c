#include "hello.h"
#include <stdio.h>

void hello(const char *name, int lucky)
{
    printf("Hello, %s! Your lucky number is %d. (from the guest)\n", name, lucky);
}

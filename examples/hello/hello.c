#include "hello.h"

#include <stdio.h>

void hello(const char *name, int lucky) {
    printf("Hello, %s! Have a wonderful day. Your lucky number is %d.\n", name, lucky);
    fflush(stdout);
}

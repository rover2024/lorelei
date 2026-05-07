#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "a.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <algo> <times> <lib_times>\n", argv[0]);
        return 0;
    }

    char *algo = argv[1];
    int times = atoi(argv[2]);
    int lib_times = atoi(argv[3]);

    set_times(lib_times);

    double res = 0;
    if (strcmp("sqrt", algo) == 0) {
        for (int i = 0; i < times; i++) {
            res = sqrt_A(i + 1);
            consume(res);
        }
    } else if (strcmp("exp", algo) == 0) {
        for (int i = 0; i < times; i++) {
            res = exp_A((i + 1) % 20);
            consume(res);
        }
    } else if (strcmp("log", algo) == 0) {
        for (int i = 0; i < times; i++) {
            res = log_A(i + 1);
            consume(res);
        }
    } else if (strcmp("cos", algo) == 0) {
        for (int i = 0; i < times; i++) {
            res = cos_A(i + 1);
            consume(res);
        }
    } else if (strcmp("sin", algo) == 0) {
        for (int i = 0; i < times; i++) {
            res = sin_A(i + 1);
            consume(res);
        }
    } else if (strcmp("tan", algo) == 0) {
        for (int i = 0; i < times; i++) {
            res = tan_A(i + 1);
            consume(res);
        }
    } else {
        printf("Unknown algo: %s\n", algo);
        return 1;
    }

    printf("res: %f\n", res);
    return 0;
}

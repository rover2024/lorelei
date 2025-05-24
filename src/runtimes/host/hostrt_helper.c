#include "hostrt_p.h"

#include <stdio.h>
#include <stdlib.h>

#include "loreshared.h"

static void HostHelper_ExtractPrintFArgs(void **args, void *ret) {
    (void) ret;

    // ? va_list
    // Lore_ExtractPrintFArgs((char *) args[0], (void *) args[1], (struct LORE_VARG_ENTRY *) args[2]);

    fprintf(stderr, "Not implemented yet\n");
    abort();
}

static void HostHelper_ExtractSScanFArgs(void **args, void *ret) {
    (void) ret;
    // Lore_ExtractSScanFArgs((char *) args[0], (char *) args[1], (void *) args[2], (struct LORE_VARG_ENTRY *) args[3]);

    fprintf(stderr, "Not implemented yet\n");
    abort();
}

typedef void (*FP_HostHelper)(void **args, void *ret);

static FP_HostHelper hostHelpers[] = {
    HostHelper_ExtractPrintFArgs,
    HostHelper_ExtractSScanFArgs,
};

void Lore_HostHelper(int id, void **args, void *ret) {
    hostHelpers[id](args, ret);
}
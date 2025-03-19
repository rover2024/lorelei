#include "hostrt_p.h"

#include "loreshared.h"

static void HostHelper_ExtractPrintFArgs(void **args, void *ret) {
    (void) ret;
    Lore_ExtractPrintFArgs((char *) args[0], (void *) args[1], (struct LORE_VARG_ENTRY *) args[2]);
}

static void HostHelper_ExtractSScanFArgs(void **args, void *ret) {
    (void) ret;
    Lore_ExtractSScanFArgs((char *) args[0], (char *) args[1], (void *) args[2], (void **) args[3]);
}

typedef void (*FP_HostHelper)(void **args, void *ret);

static FP_HostHelper hostHelpers[] = {
    HostHelper_ExtractPrintFArgs,
    HostHelper_ExtractSScanFArgs,
};

void Lore_HostHelper(int id, void **args, void *ret) {
    hostHelpers[id](args, ret);
}
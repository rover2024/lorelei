#include <stdio.h>

#include <lorelei/loreshared.h>

// NOTICE: Exception of MacOS/Arm64 variadic calling convention
// https://github.com/python/cpython/issues/92892#issuecomment-1129675656

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; ++i) {
        struct LORE_VARG_ENTRY args[3];
        args[0].type = LORE_VT_POINTER;
        args[0].p = "%d - %s\n";
        args[1].type = LORE_VT_INT;
        args[1].i = i + 1;
        args[2].type = LORE_VT_POINTER;
        args[2].p = argv[i];

        struct LORE_VARG_ENTRY ret;
        ret.type = LORE_VT_INT;
        Lore_VariadicCall(printf, 3, args, &ret);
    }
    return 0;
}
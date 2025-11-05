#include <stdio.h>

#include <lorelei/loreshared.h>

// NOTICE: Exception of MacOS/Arm64 variadic calling convention
// https://github.com/python/cpython/issues/92892#issuecomment-1129675656

int main(int argc, char *argv[]) {
    printf("[Test PrintF]\n");
    for (int i = 0; i < argc; ++i) {
        struct CVargEntry args1[1];
        args1[0].type = CVargType_Pointer;
        args1[0].p = "%d - %s\n";

        struct CVargEntry args2[2];
        args2[0].type = CVargType_Int;
        args2[0].i = i + 1;
        args2[1].type = CVargType_Pointer;
        args2[1].p = argv[i];

        struct CVargEntry ret;
        ret.type = CVargType_Int;
        Lore_VAFmtCall(printf, 1, args1, 2, args2, &ret);
    }
    printf("\n");

    printf("[Test VPrintF]\n");
    for (int i = 0; i < argc; ++i) {
        struct CVargEntry args1[1];
        args1[0].type = CVargType_Pointer;
        args1[0].p = "%d - %s\n";

        struct CVargEntry args2[2];
        args2[0].type = CVargType_Int;
        args2[0].i = i + 1;
        args2[1].type = CVargType_Pointer;
        args2[1].p = argv[i];

        struct CVargEntry ret;
        ret.type = CVargType_Int;
        Lore_VAFmtCallV(vprintf, 1, args1, 2, args2, &ret);
    }
    return 0;
}
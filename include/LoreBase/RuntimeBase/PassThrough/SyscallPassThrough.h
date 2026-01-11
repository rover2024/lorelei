#ifndef LORE_BASE_RUNTIMEBASE_SYSCALLPASSTHROUGH_H
#define LORE_BASE_RUNTIMEBASE_SYSCALLPASSTHROUGH_H

namespace lore {

    static constexpr int SyscallPathThroughNumber = 2048;

    enum SyscallPassThroughID {
        SPID_GetHostAttribute = 1,
        SPID_LogMessage,
        SPID_LoadLibrary,
        SPID_FreeLibrary,
        SPID_GetProcAddress,
        SPID_GetErrorMessage,
        SPID_GetModulePath,
        SPID_InvokeProc,
        SPID_GetThunkInfo,
        SPID_HeapAlloc,
        SPID_HeapFree,
        SPID_SetSpecialEntry,
    };

}

#endif // LORE_BASE_RUNTIMEBASE_SYSCALLPASSTHROUGH_H

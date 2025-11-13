#ifndef HOSTSERVERAPI_H
#define HOSTSERVERAPI_H

#include <cstdint>

#include <lorelei/HostRT/Global.h>

extern "C" {

LOREHOSTRT_EXPORT uint64_t LOREHOSTRT_dispatchSyscall(uint64_t num, uint64_t a1, uint64_t a2,
                                                      uint64_t a3, uint64_t a4, uint64_t a5,
                                                      uint64_t a6);

LOREHOSTRT_EXPORT void     LOREHOSTRT_setRunTaskEntry(void *runTask);

LOREHOSTRT_EXPORT void     LOREHOSTRT_notifyThreadEntry();

LOREHOSTRT_EXPORT void     LOREHOSTRT_notifyThreadExit();

}

#endif // HOSTSERVERAPI_H

#ifndef MULTITHREADING_H
#define MULTITHREADING_H

// TODO: Find a more elegant way to handle multi-threading

#include <pthread.h>

#include <lorelei/HostRT/Global.h>

extern "C" {

LOREHOSTRT_EXPORT void LOREHOSTRT_setGetLastPThreadId(pthread_t (*getLastPThreadId)(void));

LOREHOSTRT_EXPORT int  LOREHOSTRT_pthread_create(pthread_t *thread,
                                                 const pthread_attr_t *attr,
                                                 void *(*start_routine)(void *), void *arg);

LOREHOSTRT_EXPORT void LOREHOSTRT_pthread_exit(void *ret);

}

#endif // MULTITHREADING_H

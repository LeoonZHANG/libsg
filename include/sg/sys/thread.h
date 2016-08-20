/*
 * thread.h
 * Author: wangwei.
 * Platform independent implementation of threading.
 */

#ifndef LIBSG_THREAD_H
#define LIBSG_THREAD_H

#include "os.h"

#if defined(OS_WIN)
# include <Windows.h>
#else
# include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (sg_thread_routine)(void *);

struct sg_thread {
    void                *arg;
#if defined(OS_WIN)
    HANDLE              handle;
#else
    pthread_t           handle;
#endif
    sg_thread_routine   *routine;
};

/* Start a thread and run it. */
void sg_thread_init(struct sg_thread *self, sg_thread_routine *routine, void *arg);

/* Wait a thread to exit. */
void sg_thread_join(struct sg_thread *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_THREAD_H */

/*
 * mutex.h
 * Author: wangwei.
 * Mutex for multiple platforms.
 */

#ifndef LIBSG_MUTEX_H
#define LIBSG_MUTEX_H

#include "../sys/os.h"

#ifdef OS_WIN
# include "windef.h"
#else
# include <pthread.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct sg_mutex {
#ifdef OS_WIN
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
};

/*  create mutex */
void sg_mutex_create(struct sg_mutex *self);

/*  lock the mutex */
void sg_mutex_lock(struct sg_mutex *self);

/*  unlock the mutex */
void sg_mutex_unlock(struct sg_mutex *self);

/*  destroy the mutex */
void sg_mutex_destroy(struct sg_mutex *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MUTEX_H */
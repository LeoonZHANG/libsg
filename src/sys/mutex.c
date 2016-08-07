/*
 * mutex.c
 * Author: wangwei.
 * Mutex for multiple platforms.
 */

#include "../../include/sys/mutex.h"
#include "../../include/util/assert.h"


void sg_mutex_create(struct sg_mutex *self)
{
#ifdef OS_WIN
    sg_assert(self);

    InitializeCriticalSection(&self->mutex);
#else
    int rc;

    sg_assert(self);

    rc = pthread_mutex_init(&self->mutex, NULL);
    /* sg_assert(rc == 0); */
#endif
}

void sg_mutex_lock(struct sg_mutex *self)
{
#ifdef OS_WIN
    sg_assert(self);

    EnterCriticalSection(&self->mutex);
#else
    int rc;

    sg_assert(self);

    rc = pthread_mutex_lock(&self->mutex);
    /* sg_assert(rc == 0); */
#endif
}

void sg_mutex_unlock(struct sg_mutex *self)
{
#ifdef OS_WIN
    sg_assert(self);

    LeaveCriticalSection(&self->mutex);
#else
    int rc;

    sg_assert(self);

    rc = pthread_mutex_unlock(&self->mutex);
    /* sg_assert(rc == 0); */
#endif
}

void sg_mutex_destroy(struct sg_mutex *self)
{
#ifdef OS_WIN
    sg_assert(self);

    DeleteCriticalSection(&self->mutex);
#else
    int rc;

    sg_assert(self);

    rc = pthread_mutex_destroy(&self->mutex);
    /* sg_assert(rc == 0); */
#endif
}
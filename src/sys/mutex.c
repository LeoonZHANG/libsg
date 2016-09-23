/*
 * mutex.c
 * Author: wangwei.
 * Mutex for multiple platforms.
 */

#include <stdlib.h>
#include <sg/sg.h>
#include <sg/sys/mutex.h>

#ifdef SG_OS_WINDOWS
# include <Windows.h>
#else
# include <pthread.h>
#endif

struct sg_mutex_real {
#ifdef SG_OS_WINDOWS
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
};


sg_mutex_t *sg_mutex_create(void)
{
    struct sg_mutex_real *self = (struct sg_mutex_real *)malloc(sizeof(struct sg_mutex_real));
    sg_assert(self);

#ifdef SG_OS_WINDOWS
    InitializeCriticalSection(&self->mutex);
#else
    int rc;
    rc = pthread_mutex_init(&self->mutex, NULL);
    /* sg_assert(rc == 0); */
#endif

    return (sg_mutex_t *)self;
}

void sg_mutex_lock(sg_mutex_t *self)
{
    sg_assert(self);

#ifdef SG_OS_WINDOWS
    EnterCriticalSection(&((struct sg_mutex_real *)self)->mutex);
#else
    int rc;
    rc = pthread_mutex_lock(&((struct sg_mutex_real *)self)->mutex);
    /* sg_assert(rc == 0); */
#endif
}

void sg_mutex_unlock(sg_mutex_t *self)
{
    sg_assert(self);

#ifdef SG_OS_WINDOWS
    LeaveCriticalSection(&((struct sg_mutex_real *)self)->mutex);
#else
    int rc;
    rc = pthread_mutex_unlock(&((struct sg_mutex_real *)self)->mutex);
    /* sg_assert(rc == 0); */
#endif
}

void sg_mutex_destroy(sg_mutex_t *self)
{
    sg_assert(self);

#ifdef SG_OS_WINDOWS
    DeleteCriticalSection(&((struct sg_mutex_real *)self)->mutex);
#else
    int rc;
    rc = pthread_mutex_destroy(&((struct sg_mutex_real *)self)->mutex);
    /* sg_assert(rc == 0); */
#endif

    free(self);
}
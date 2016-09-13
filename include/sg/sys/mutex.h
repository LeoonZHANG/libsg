/*
 * mutex.h
 * Author: wangwei.
 * Mutex for multiple platforms.
 */

#ifndef LIBSG_MUTEX_H
#define LIBSG_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_mutex_real sg_mutex_t;

/*  create mutex */
sg_mutex_t *sg_mutex_create(void);

/*  lock the mutex */
void sg_mutex_lock(sg_mutex_t *self);

/*  unlock the mutex */
void sg_mutex_unlock(sg_mutex_t *self);

/*  destroy the mutex */
void sg_mutex_destroy(sg_mutex_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MUTEX_H */

/**
 * thread.h
 * Platform independent implementation of threading.
 */

#ifndef LIBSG_THREAD_H
#define LIBSG_THREAD_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (sg_thread_routine_func_t)(void *);

typedef struct sg_thread_real sg_thread_t;

/* Start a thread and run it. */
sg_thread_t *sg_thread_alloc(sg_thread_routine_func_t *routine, void *arg);

/* Wait a thread to exit. */
void sg_thread_join(sg_thread_t *self);

void sg_thread_free(sg_thread_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_THREAD_H */

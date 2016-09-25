/**
 * sem.h
 * Cross-platform semaphore.
 * 从https://github.com/nanomsg/nanomsg/blob/master/src/utils/sem.c拷贝
 */

#ifndef LIBSG_SEM_H
#define LIBSG_SEM_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_sem_real sg_sem_t;

sg_sem_t *sg_sem_open(void);

/*  Unlock the semaphore. */
void sg_sem_post(sg_sem_t *self);

/*  Waits till sem object becomes unlocked and locks it. */
int sg_sem_wait(sg_sem_t *self);

void sg_sem_close(sg_sem_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SEM_H */
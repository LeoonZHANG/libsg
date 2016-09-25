/**
 * spinlock.h
 *
 */
#ifndef LIBSG_SPINLOCK_H
#define LIBSG_SPINLOCK_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_spinlock_real sg_spinlock_t;

sg_spinlock_t *sg_spinlock_alloc(void);

bool sg_spinlock_lock(sg_spinlock_t *self);

bool sg_spinlock_unlock(sg_spinlock_t *self);

void sg_spinlock_free(sg_spinlock_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SPINLOCK_H */

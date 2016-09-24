/**
 * shm.h
 * Shared memory for multiple platforms.
 */

#ifndef LIBSG_SHM_H
#define LIBSG_SHM_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_shm_real sg_shm_t;

sg_shm_t *sg_shm_open(const char *name);

uint32_t sg_shm_read(sg_shm_t *self, uint8_t *buf, uint32_t try_size);

bool sg_shm_write(sg_shm_t *self, uint8_t *data, uint32_t size);

void sg_shm_close(sg_shm_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SHM_H */

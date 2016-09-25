/**
 * shm.h
 * Shared memory for multiple platforms.
 * 参考 https://github.com/apache/apr/blob/trunk/include/apr_shm.h
 * 接口需要参照上面的链接调整,基本可以扒下来, 注意,把平台特有对shm接口去掉,比如apr_shm_delete
 */

#ifndef LIBSG_SHM_H
#define LIBSG_SHM_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_shm_real sg_shm_t;

sg_shm_t *sg_shm_create(const char *filename, uint64_t require_size);

sg_shm_t *sg_shm_attach(const char *filename);

uint64_t sg_shm_get_size(sg_shm_t *self);

void *sg_shm_get_base_addr(sg_shm_t *self);

bool sg_shm_remove(const char *filename);

void sg_shm_detach(sg_shm_t *self);

void sg_shm_detach(sg_shm_t *self);

void sg_shm_destroy(sg_shm_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_SHM_H */

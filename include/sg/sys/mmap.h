/**
 * mmap.h
 * 调用https://github.com/apache/apr/blob/trunk/include/apr_mmap.h
 * 注意, mmap没有用过,下面的接口可能也不是深思熟虑的,请酌情调整
 */

#ifndef LIBSG_MMAP_H
#define LIBSG_MMAP_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum sg_mmap_flag {
    SGMMAPFLAG_R = 0,
    SGMMAPFLAG_W,
    SGMMAPFLAG_RW
};

typedef struct sg_mmap_real sg_mmap_t;

sg_mmap_t sg_mmap_create(FILE *file, uint64_t offset, uint64_t size, enum sg_mmap_flag flag);

sg_mmap_t sg_mmap_dup(sg_mmap_t *self);

void *sg_mmap_get_addr(sg_mmap_t *self);

void sg_mmap_delete(sg_mmap_t *self);

void sg_mmap_offset(sg_mmap_t *self, int offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MMAP_H */

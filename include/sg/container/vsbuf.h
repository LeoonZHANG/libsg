/**
 * vsbuf.h
 * Variable-sized buffer.
 */

#ifndef LIBSG_VSBUF_H
#define LIBSG_VSBUF_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_vsbuf {
    void   *mem;
    size_t size;
} sg_vsbuf_t;

sg_vsbuf_t *sg_vsbuf_create(void);

int sg_vsbuf_insert(sg_vsbuf_t *buf, const void *mem, size_t size);

void sg_vsbuf_reset(sg_vsbuf_t *buf);

void sg_vsbuf_destroy(sg_vsbuf_t *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_VSBUF_H */

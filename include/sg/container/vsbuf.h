/*
 * vsbuf.h
 * Author: wangwei.
 * Flexible variable length buffer.
 */

#ifndef LIBSG_VLBUF_H
#define LIBSG_VLBUF_H

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

#endif /* LIBSG_VLBUF_H */

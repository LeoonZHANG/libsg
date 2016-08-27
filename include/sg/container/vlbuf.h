/*
 * vlbuf.h
 * Author: wangwei.
 * Flexible variable length buffer.
 */

#ifndef LIBSG_VLBUF_H
#define LIBSG_VLBUF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_vlbuf {
    void *mem;
    size_t size;
} sg_vlbuf_t;

sg_vlbuf_t *sg_vlbuf_create(void);

int sg_vlbuf_insert(sg_vlbuf_t *buf, const void *mem, size_t size);

void sg_vlbuf_reset(sg_vlbuf_t *buf);

void sg_vlbuf_destroy(sg_vlbuf_t *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_VLBUF_H */

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

struct sg_flex_buf {
    void *mem;
    size_t size;
};

struct sg_flex_buf *sg_flex_buf_create(void);

struct sg_flex_buf *sg_flex_buf_create2(size_t initial_size);

int sg_flex_buf_insert(struct sg_flex_buf *buf, const void *mem, size_t size);

void sg_flex_buf_reset(struct sg_flex_buf *buf);

void sg_flex_buf_destroy(struct sg_flex_buf *buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_FLEX_BUF_H */

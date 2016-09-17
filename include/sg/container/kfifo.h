/**
 * kfifo.h
 * linux内核的kfifo数据结构
 */

#ifndef LIBSG_KFIFO_H
#define LIBSG_KFIFO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_kfifo_real sg_kfifo_t;

sg_kfifo_t *fifo_alloc(uint64_t size);
void fifo_free(sg_kfifo_t *);

bool kfifo_put(sg_kfifo_t *, const void *buf, uint64_t len);
bool kfifo_get(sg_kfifo_t *, void *out_buf, uint64_t out_buf_len, uint64_t *real_out_len);/*这个接口的参数可能要根据实际情况调整,尽量不做memcpy*/
uint64_t kfifo_len(struct xy3_fifo * fifo);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_KFIFO_H */

/**
 * mq.h
 * Message queue for lockless share resource and IPC only.
 * 利用操作系统的消息队列,避免使用锁等其它数据同步方法
 * 接口可能需要调整
 */

#ifndef LIBSG_MQ_H
#define LIBSG_MQ_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_mq_real sg_mq_t;

sg_mq_t *sg_mq_open(void);

bool sg_mq_send(sg_mq_t *self, void *data, uint64_t size);

bool sg_mq_recv(sg_mq_t *self, ...);

void sg_mq_close(sg_mq_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_MQ_H */
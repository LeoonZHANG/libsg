/**
 * enet.h
 * ENET is a reliable transfer protocol based on UDP.
 * This is enet client.
 */

#ifndef LIBSG_ENET_H
#define LIBSG_ENET_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct enet_real sg_enet_t;

typedef void (*sg_enet_on_open_func_t)(sg_enet_t *);
/* WARNING: next callback will be called after previous is done,
   don't cost too much time for this callback */
typedef void (*sg_enet_on_recv_func_t)(sg_enet_t *, char *data, size_t size); /* 如果回调中操作阻塞时间长,是否影响其他操作以及下一次本回调执行 */
typedef void (*sg_enet_on_sent_func_t)(sg_enet_t *, int status/*0:OK*/, void *data, size_t len);
typedef void (*sg_enet_on_close_func_t)(sg_enet_t *, int code, const char *reason);
typedef void (*sg_enet_on_error_func_t)(sg_enet_t *, const char *msg);

sg_enet_t *sg_enet_open(const char *server_addr, int server_port,
                      sg_enet_on_open_func_t,
                      sg_enet_on_recv_func_t,
                      sg_enet_on_sent_func_t,
                      sg_enet_on_close_func_t);

int sg_enet_run(sg_enet_t *, int interval_ms);

sg_err_t sg_enet_send(sg_enet_t *, const void *data, uint64_t size);

/* 限制客户端发送速度, kbps为0不做任何限制 */
//void sg_enet_set_max_send_speed(sg_enet_t *, size_t kbps);

//int sg_enet_get_speed(sg_enet_t *, size_t *send_kbps, size_t *recv_kbps);

void sg_enet_close(sg_enet_t *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ENET_H */
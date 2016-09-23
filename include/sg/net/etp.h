/**
 * etp.h
 * ETP (extreme transfer protocol) is a transfer protocol based on LIBUV / UDP / KCP and FEC.
 * This is etp client.
 */

#ifndef LIBSG_ETP_H
#define LIBSG_ETP_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct etp_comm_client_real sg_etp_t;

typedef void (*sg_etp_on_open_func_t)(sg_etp_t *);
/* WARNING: next callback will be called after previous is done,
   don't cost too much time for this callback */
typedef void (*sg_etp_on_recv_func_t)(sg_etp_t *, char *data, size_t size); /* 如果回调中操作阻塞时间长,是否影响其他操作以及下一次本回调执行 */
typedef void (*sg_etp_on_sent_func_t)(sg_etp_t *, int status/*0:OK*/, void *data, size_t len);
typedef void (*sg_etp_on_close_func_t)(sg_etp_t *, int code, const char *reason);
typedef void (*sg_etp_on_error_func_t)(sg_etp_t *, const char *msg);

sg_etp_t *sg_etp_open(const char *server_addr, int server_port,
                      sg_etp_on_open_func_t,
                      sg_etp_on_recv_func_t,
                      sg_etp_on_sent_func_t,
                      sg_etp_on_close_func_t);

int sg_etp_run(sg_etp_t *, int interval_ms);

sg_err_t sg_etp_send(sg_etp_t *, const void *data, uint64_t size);

/* 限制客户端发送速度, kbps为0不做任何限制 */
void sg_etp_set_max_send_speed(sg_etp_t *, size_t kbps);

int sg_etp_get_speed(sg_etp_t *, size_t *send_kbps, size_t *recv_kbps);

void sg_etp_close(sg_etp_t *);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ETP_H */
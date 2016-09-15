/**
 * quic.h
 * Chromium quic client wrapper.
 */

#ifndef LIBSG_QUIC_H
#define LIBSG_QUIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_quic_real sg_quic_t;

typedef void (*sg_quic_on_open_func_t)(sg_quic_t *);
typedef void (*sg_quic_on_recv_func_t)(sg_quic_t *, char *data, size_t size);
typedef void (*sg_quic_on_sent_func_t)(sg_quic_t *, int status/*0:OK*/, void *data, size_t len);
typedef void (*sg_quic_on_close_func_t)(sg_quic_t *, int code, const char *reason);
typedef void (*sg_quic_on_error_func_t)(sg_quic_t *, const char *msg);

int sg_quic_init(void);

sg_quic_t *sg_quic_open(const char *server_addr, int server_port,
                      sg_quic_on_open_func_t,
                      sg_quic_on_recv_func_t,
                      sg_quic_on_sent_func_t,
                      sg_quic_on_close_func_t);

int sg_quic_run(sg_quic_t *, int interval_ms);

int sg_quic_send(sg_quic_t *, const void *data, uint64_t size);

/* 限制客户端发送速度, kbps为0不做任何限制 */
void sg_quic_set_max_send_speed(sg_quic_t *, size_t kbps);

int sg_quic_get_speed(sg_quic_t *, size_t * send_kbps, size_t * recv_kbps);

void sg_quic_close(sg_quic_t *);

void sg_quic_free(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_QUIC_H */
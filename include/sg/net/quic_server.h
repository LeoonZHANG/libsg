/**
 * quic_server.h
 * Chromium quic server wrapper.
 */

#ifndef LIBSG_QUIC_SERVER_H
#define LIBSG_QUIC_SERVER_H

#include <sg/sg.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct sg_quic_client_real sg_quic_client_t;
typedef struct sg_quic_server_real sg_quic_server_t;

typedef void (*sg_quic_server_on_iter_client_func_t)(sg_quic_server_t *, sg_quic_client_t *, void *ctx);
typedef void (*sg_quic_server_on_open_func_t)(sg_quic_client_t *);
typedef void (*sg_quic_server_on_recv_func_t)(sg_quic_client_t *, char *data, size_t len);
typedef void (*sg_quic_server_on_sent_func_t)(sg_quic_client_t *, int status/*0:OK*/, void *data, size_t len);
typedef void (*sg_quic_server_on_error_func_t)(sg_quic_client_t *, const char *msg);
typedef void (*sg_quic_server_on_close_func_t)(sg_quic_client_t *, int code, const char *msg);


int sg_quic_server_init(void);
sg_quic_server_t *sg_quic_server_open(
        const char                     *server_addr,
        int                             server_port,
        int                             max_backlog,
        sg_quic_server_on_open_func_t,
        sg_quic_server_on_recv_func_t,
        sg_quic_server_on_sent_func_t,
        sg_quic_server_on_error_func_t,
        sg_quic_server_on_close_func_t);
void sg_quic_server_run(sg_quic_server_t *, int interval_ms); /* 实际上就是调用了uv_run之类的接口，让线程保持阻塞，直至quic_server结束时此接口才返回 */
int sg_quic_server_send(sg_quic_client_t *, void *data, size_t size);
int sg_quic_server_get_conn_size(sg_quic_server_t *);
void sg_quic_server_set_max_conn(sg_quic_server_t *, int); /* 设置最大连接数， 如果是0表示不受限制， 因为超过此参数导致拒绝新客户连接时，请在日志中输出日志 */
int sg_quic_server_get_speed(sg_quic_server_t *, size_t * send_kbps, size_t * recv_kbps);
int sg_quic_server_iter_client(sg_quic_server_t *, sg_quic_server_on_iter_client_func_t, void *ctx);
int sg_quic_server_get_client_info(sg_quic_client_t *, char **addr, int *port);
int sg_quic_server_get_client_speed(sg_quic_client_t *, size_t * send_kbps, size_t * recv_kbps);
void sg_quic_server_set_client_max_send_speed(sg_quic_client_t *, size_t kbps); /* 限制对此客户端的发送速度 */
void sg_quic_server_close_client(sg_quic_client_t *);
void sg_quic_server_close(sg_quic_server_t *);
void sg_quic_server_free(void);

#ifdef  __cplusplus
}
#endif

#endif  /* LIBSG_QUIC_SERVER_H */


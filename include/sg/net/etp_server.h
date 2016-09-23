/**
 * etp_server.h
 */

#ifndef LIBSG_ETP_SERVER_H
#define LIBSG_ETP_SERVER_H

#include <sg/sg.h>
#include <sg/str/vsstr.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct etp_comm_client_real sg_etp_client_t;
typedef struct sg_etp_server_real sg_etp_server_t;

typedef void (*sg_etp_server_on_iter_client_func_t)(sg_etp_server_t *, sg_etp_client_t *, void *ctx);
typedef void (*sg_etp_server_on_open_func_t)(sg_etp_client_t *);
/* WARNING: next callback will be called after previous is done,
   don't cost too much time for this callback */
typedef void (*sg_etp_server_on_recv_func_t)(sg_etp_client_t *, char *data, size_t len);
typedef void (*sg_etp_server_on_sent_func_t)(sg_etp_client_t *, int status/*0:OK*/, void *data, size_t len);
typedef void (*sg_etp_server_on_error_func_t)(sg_etp_client_t *, const char *msg);
typedef void (*sg_etp_server_on_close_func_t)(sg_etp_client_t *, int code, const char *msg);


sg_etp_server_t *
sg_etp_server_open(const char *server_addr, int server_port, int max_backlog,
        sg_etp_server_on_open_func_t on_open, sg_etp_server_on_recv_func_t on_recv,
        sg_etp_server_on_sent_func_t on_sent, sg_etp_server_on_error_func_t on_error,
        sg_etp_server_on_close_func_t on_close);
void sg_etp_server_run(sg_etp_server_t *self, int interval_ms); /* 实际上就是调用了uv_run之类的接口，让线程保持阻塞，直至etp_server结束时此接口才返回 */
sg_err_t sg_etp_server_send(sg_etp_client_t *client, void *data, size_t size);
int sg_etp_server_get_conn_size(sg_etp_server_t *self);
void sg_etp_server_set_max_conn(sg_etp_server_t *self, int); /* 设置最大连接数， 如果是0表示不受限制， 因为超过此参数导致拒绝新客户连接时，请在日志中输出日志 */
int sg_etp_server_get_speed(sg_etp_server_t *self, size_t *send_kbps, size_t *recv_kbps);
int sg_etp_server_iter_client(sg_etp_server_t *self, sg_etp_server_on_iter_client_func_t, void *ctx);
sg_err_t sg_etp_server_get_client_addr(sg_etp_client_t *client, sg_vsstr_t *addr_out);
int sg_etp_server_get_client_speed(sg_etp_client_t *client, size_t *send_kbps, size_t *recv_kbps);
void sg_etp_server_set_client_max_send_speed(sg_etp_client_t *client, size_t kbps); /* 限制对此客户端的发送速度 */
void sg_etp_server_close_client(sg_etp_client_t *client);
void sg_etp_server_close(sg_etp_server_t *self);

#ifdef  __cplusplus
}
#endif

#endif  /* LIBSG_ETP_SERVER_H */

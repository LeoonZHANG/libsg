/**
 * tcp_server.h
 * High performance tcp server library based on libuv.
 */

#ifndef LIBSG_TCP_SERVER_H
#define	LIBSG_TCP_SERVER_H

#ifdef	__cplusplus
extern "C"
{
#endif

typedef struct sg_tcp_client_real sg_tcp_client_t;
typedef struct sg_tcp_server_real sg_tcp_server_t;

typedef void (*sg_tcp_server_on_iter_client_func_t)(sg_tcp_server_t *, sg_tcp_client_t *, void *ctx);

typedef void (*sg_tcp_server_on_open_func_t)(sg_tcp_client_t *);
typedef void (*sg_tcp_server_on_data_func_t)(sg_tcp_client_t *, char *data, size_t size);
typedef void (*sg_tcp_server_on_sent_func_t)(sg_tcp_client_t *, int status/*0:OK*/, void *data, size_t len);
typedef void (*sg_tcp_server_on_error_func_t)(sg_tcp_client_t *, const char *msg);
typedef void (*sg_tcp_server_on_close_func_t)(sg_tcp_client_t *, int code, const char *msg);

sg_tcp_server_t *tcp_server_open(const char *server_addr, /* 同步接口，server监听成功则返回0，否则返回负数 */
        int server_port,
        int max_backlog,
        sg_tcp_server_on_open_func_t,
        sg_tcp_server_on_data_func_t,
        sg_tcp_server_on_sent_func_t,
        sg_tcp_server_on_error_func_t,
        sg_tcp_server_on_close_func_t);
void sg_tcp_server_run(sg_tcp_server_t *);
void sg_tcp_server_send(sg_tcp_client_t *, void *data, size_t size); /*发送数据 异步接口*/
int sg_tcp_server_get_conn_size(sg_tcp_server_t *);
void sg_tcp_server_set_max_conn(sg_tcp_server_t *, int); /* 设置最大连接数， 如果是0表示不受限制， 因为超过此参数导致拒绝新客户连接时，请在日志中输出日志 */
int sg_tcp_server_get_speed(sg_tcp_server_t *, size_t &send_kbps, size_t &recv_kbps);
int sg_tcp_server_iter_client(sg_tcp_server_t *, sg_tcp_server_on_iter_client_func_t, void *ctx);
int sg_tcp_server_get_client_info(sg_tcp_client_t *, char **ip, int *port);
int sg_tcp_server_get_client_speed(sg_tcp_client_t *, size_t &send_kbps, size_t &recv_kbps);
void sg_etp_server_set_client_max_send_speed(sg_tcp_client_t *, size_t kbps);
void sg_tcp_server_close_client(sg_tcp_client_t *);
void sg_tcp_server_close(sg_tcp_server_t *);

#ifdef	__cplusplus
}
#endif

#endif	/* LIBSG_TCP_SERVER_H */


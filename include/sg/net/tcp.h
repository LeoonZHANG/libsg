/**
 * tcp.h
 * High performance tcp client library based on libuv.
 */

#ifndef LIBSG_TCP_H
#define	LIBSG_TCP_H

#ifdef	__cplusplus
extern "C"
{
#endif

typedef struct sg_tcp_real sg_tcp_t;

typedef void (*sg_tcp_on_open_func_t)(sg_tcp_t *);
typedef void (*sg_tcp_on_data_func_t)(sg_tcp_t *, char *data, size_t size);
typedef void (*sg_tcp_on_sent_func_t)(sg_tcp_t *, int status/*0:OK*/);
typedef void (*sg_tcp_on_close_func_t)(sg_tcp_t *, int code, const char *reason);

sg_tcp_t *sg_tcp_open(const char *server_addr, int server_port,
        sg_tcp_on_open_func_t, sg_tcp_on_data_func_t,
        sg_tcp_on_sent_func_t, sg_tcp_on_close_func_t);
void tcp_client_run(sg_tcp_t *);
void tcp_client_send_data(sg_tcp_t *, void *data, size_t size);
/* 限制客户端发送速度, kbps为0不做任何限制 */
void sg_tcp_set_max_send_speed(sg_etp_t *, size_t kbps);
int sg_tcp_get_speed(sg_etp_t *, size_t &send_kbps, size_t &recv_kbps);
void sg_tcp_close(sg_tcp_t *);

#ifdef	__cplusplus
}
#endif

#endif	/* LIBSG_TCP_H */


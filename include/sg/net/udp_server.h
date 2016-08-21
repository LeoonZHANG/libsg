/**
 * udp_server.h
 * High performance udp server library based on libuv.
 */

#ifndef LIBSG_UDP_SERVER_H
#define LIBSG_UDP_SERVER_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sg_udp_client_real sg_udp_client_t;

typedef struct sg_udp_server_real sg_udp_server_t;

/*
flags is zero: everything is ok.
flags is UV_UDP_PARTIAL(2): 当前系统UDP接收缓冲区太小, 接收到的报文已经被截断, 用户收到的报文是部分报文, 需要调节UDP socket缓冲区大小.
*/
typedef void (*sg_udp_server_on_recv_func_t)(sg_udp_server_t *, sg_udp_client_t *, void *data, size_t size, void *ctx);

/* ip: "0.0.0.0" means listen all local address */
sg_udp_server_t *
sg_udp_server_open(const char *ip, int port, sg_udp_server_on_recv_func_t, void *ctx);
/* Sync api to loop udp server.*/
void sg_udp_server_run(sg_udp_server_t *);
/* If data_realloc is true, api will re malloc buffer for data. */
int sg_udp_server_send(sg_udp_client_t *, const void *data, size_t size, bool data_realloc);
int sg_udp_server_get_conn_size(sg_udp_server_t *); /* udp server需要这个接口吗？ */
void sg_udp_server_set_max_conn(sg_udp_server_t *, int); /* udp server需要这个接口吗？ */
int sg_udp_server_get_speed(sg_udp_server_t *, size_t &send_kbps, size_t &recv_kbps);
int sg_udp_server_get_client_info(sg_udp_client_t *, char **ip, int *port);
int sg_udp_server_get_client_speed(sg_udp_client_t *, size_t &send_kbps, size_t &recv_kbps);
void sg_udp_server_set_client_max_send_speed(sg_udp_client_t *, size_t kbps);/* 限制对此客户端的发送速度 */
void sg_udp_server_close_client(sg_udp_client_t *);
void sg_udp_server_close(sg_udp_server_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_UDP_SERVER_H */

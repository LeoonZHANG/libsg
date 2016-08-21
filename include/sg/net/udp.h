/**
 * udp.h
 * High performance udp client library based on libuv.
 */

#ifndef LIBSG_UDP_H
#define LIBSG_UDP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sg_udp_real {
    struct sockaddr *server_sock_addr;
    char *server_ip;
    int server_port;
    sg_udp_on_recv_func_t on_recv;
    void *ctx;
}sg_udp_t;

/*
flags is zero: everything is ok.
flags is UV_UDP_PARTIAL(2): 当前系统UDP接收缓冲区太小, 接收到的报文已经被截断, 用户收到的报文是部分报文, 需要调节UDP socket缓冲区大小.
*/
typedef void (*sg_udp_on_recv_func_t)(sg_udp_t *, void *data, size_t size, unsigned int flags, void *ctx);

sg_udp_t *sg_udp_open(const char *server_addr, int server_port, sg_udp_on_recv_func_t, void *ctx);

/* Sync api to loop udp client.*/
void sg_udp_run(sg_udp_t *);

/* If data_realloc is true, api will re malloc buffer for data. */
int sg_udp_send(sg_udp_t *, const void *data, size_t size, bool data_realloc);

/* 限制客户端发送速度, kbps为0不做任何限制 */
void sg_udp_set_max_send_speed(sg_udp_t *, size_t kbps);

int sg_udp_get_speed(sg_udp_t *, size_t &send_kbps, size_t &recv_kbps);

int sg_udp_set_broadcast(sg_udp_t *, bool on);

void sg_udp_close(sg_udp_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_UDP_H */

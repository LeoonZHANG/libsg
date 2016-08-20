/**
 * udp.h
 * High performance udp client and server library based on libuv.
 */

#ifndef LIBSG_UDP_H
#define LIBSG_UDP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sg_udp_real sg_udp_t;

/*
flags is zero: everything is ok.
flags is UV_UDP_PARTIAL(2): 当前系统UDP接收缓冲区太小, 接收到的报文已经被截断, 用户收到的报文是部分报文, 需要调节UDP socket缓冲区大小.
*/
typedef void (*sg_udp_on_recv_func_t)(sg_udp_t *, void *data, size_t size, unsigned int flags, void *ctx);

sg_udp_t *
sg_udp_open_client(sg_udp_on_recv_func_t, void *ctx);

/* ip: "0.0.0.0" means listen all local address */
sg_udp_t *
sg_udp_open_server(const char *ip, int port, sg_udp_on_recv_func_t, void *ctx);

/* Sync api to loop udp client or server.*/
void
sg_udp_run(sg_udp_t *);

/* If data_realloc is true, api will re malloc buffer for data. */
int
sg_udp_send(sg_udp_t *, const struct sockaddr *, const void *data, size_t size, bool data_realloc);

int
sg_udp_send2(sg_udp_t *, const char *ip, int port, const void *data, size_t size, bool data_realloc);

int
sg_udp_set_broadcast(sg_udp_t *, bool on);

void
sg_udp_close(sg_udp_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_UDP_H */

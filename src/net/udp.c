/*
 * udp.c
 * Author: wangwei.
 * High performance udp client and server library based on libuv.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <uv.h>
#include "../../include/util/assert.h"
#include "../../include/net/udp.h"

struct sg_udp_real {
    uv_loop_t *uvloop;
    uv_udp_t  uvudp;
    sg_udp_on_recv_func_t on_recv;
    void *ctx;
};

void uv_on_alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (char *)malloc(suggested_size);
	buf->len  = buf->base ? suggested_size : 0;
}

static void uv_on_udp_recv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned int flags) {
	sg_udp_t *u = (sg_udp_t *)handle->data;

	/* 如果报文的长度为0, 且addr不是NULL, 说明收到空的UDP报文(只有UDP头, 没有UDP报文内容)也是正常的报文
	当收到UDP报文后, 如果数据全部收完后, libuv会继续调用收取数据回调, 此时nread值为0, addr为NULL, 用于指示本次UDP报文已经收取完毕
	*/
	if (nread >= 0) {
		if (nread > 0 && u->on_recv)
			u->on_recv(u, buf->base, nread, addr, flags);
			free(buf->base);
	} else
		fprintf(stderr, "uv_on_udp_recv error, nread = %zd, addr = %p ...\n", nread, addr);
}

sg_udp_t *sg_udp_open_client(sg_udp_on_recv_func_t on_recv, void *ctx)
{
	struct sg_udp_real *client;

	client = (struct sg_udp_real *)malloc(sizeof(struct sg_udp_real));
	if (!client)
		return NULL;

	client->on_recv = on_recv;
	client->ctx = ctx;
	client->uvloop = uv_default_loop();
	uv_udp_init(client->uvloop, &client->uvudp); /* init udp */
	client->uvudp.data = client;

	/* default bind to local random port number for UDP client */
	uv_udp_recv_start(&client->uvudp, uv_on_alloc_buf, uv_on_udp_recv); /* Start to receive data. */

	return (sg_udp_t *)client;
}

sg_udp_t *sg_udp_open_server(const char* ip, int port, sg_udp_on_recv_func_t on_recv, void *ctx)
{
	struct sg_udp_real *server;
	int retval;

	sg_assert(ip);

	server = (struct sg_udp_real *)malloc(sizeof(struct sg_udp_real));
	if (!server)
		return NULL;

	server->on_recv = on_recv;
	server->ctx = ctx;
	server->uvloop = uv_default_loop();
	uv_udp_init(server->uvloop, &server->uvudp); /* init udp */
	server->uvudp.data = server;

	/* bind to local ip and port for UDP server */
        if (strchr(ip, ':')) { /* ipv6 */
            struct sockaddr_in6 addr;
            uv_ip6_addr(ip, port, &addr);
            retval = uv_udp_bind(&server->uvudp, (const struct sockaddr *)&addr, 0);
        } else { /* ipv4 */
            struct sockaddr_in addr;
            uv_ip4_addr(ip, port, &addr);
            retval = uv_udp_bind(&server->uvudp, (const struct sockaddr *)&addr, 0);
        }
        if(retval >= 0)
		fprintf(stdout, "udp bind on %s:%d OK\n", ip, port);
        else {
		fprintf(stdout, "udp bind on %s:%d error\n", ip, port);;
		uv_close((uv_handle_t*) &server->uvudp, NULL);
		return -1;
        }

	uv_udp_recv_start(&server->uvudp, uv_on_alloc_buf, uv_on_udp_recv); /* Start to receive data. */

	return 0;
}

static void uv_after_udp_send(uv_udp_send_t *req, int status)
{
	free(req);
}

void sg_udp_run(sg_udp_t *u)
{
	sg_assert(u);

	uv_run(((struct sg_udp_real *)u)->uvloop, UV_RUN_DEFAULT);
}

int sg_udp_send(sg_udp_t *u, const struct sockaddr *addr, const void *data, size_t size, bool data_realloc)
{
	uv_udp_send_t *req;
	uv_buf_t buf;

	if (data_realloc) {
		req = (uv_udp_send_t *)malloc(sizeof(uv_udp_send_t) + size);
		buf = uv_buf_init((char*)req + sizeof(uv_udp_send_t), size);
		memcpy(buf.base, data, size); /* copy data to the end of req */
	} else {
		req = (uv_udp_send_t *)malloc(sizeof(uv_udp_send_t)); /* Free it when uv_after_udp_send called. */
		buf = uv_buf_init((char *)data, size);
	}
	/*req->data = (void *)client;/*?*/

	return uv_udp_send(req, &((struct sg_udp_real *)u)->uvudp, &buf, 1, addr, uv_after_udp_send);
}

int sg_udp_send2(sg_udp_t *u, const char *ip, int port, const void *data, size_t size, bool data_realloc)
{
	sg_assert(u);
	sg_assert(ip);

	if(strchr(ip, ':')) { /* ipv6 */
		struct sockaddr_in6 addr;
		uv_ip6_addr(ip, port, &addr);
		return sg_udp_send(u, (const struct sockaddr*)&addr, data, size, data_realloc);
	} else { /* ipv4 */
		struct sockaddr_in addr;
		uv_ip4_addr(ip, port, &addr);
		return sg_udp_send(u, (const struct sockaddr*)&addr, data, size, data_realloc);
	}
}

int sg_udp_set_broadcast(sg_udp_t *u, bool on)
{
	return uv_udp_set_broadcast(&((struct sg_udp_real *)u)->uvudp, on ? 1 : 0);
}

void sg_udp_close(sg_udp_t *u)
{
	sg_assert(u);

	uv_udp_recv_stop(&((struct sg_udp_real *)u)->uvudp);
	uv_close((uv_handle_t *)&((struct sg_udp_real *)u)->uvudp, NULL);
}

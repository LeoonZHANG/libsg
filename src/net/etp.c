/**
 * etp.c
 * This is the client routin for kcp protocol with libuv, using UDP for
 * the data transfer protocol.
 */

#include "uv.h"
#include "ikcp.h"
#include "etp_client_shared.h"
#include <sg/sg.h>
#include <sg/net/etp.h>
#include <sg/net/etp_private.h>

/* ENSURE: interval是用在libuv的loop时间间隔上的, 为什么又用在kcp的接口上了, 可以不同吗 */

/* FIXME: any possible to make the same conversion id in two host? */
static inline IUINT32 __make_kcp_conv_id(void)
{
    struct timeval tv;

    /* don't use client pointer as conversion id, because,
       client pointer could be the same if open this process twice */
    gettimeofday(&tv, NULL);
    return (IUINT32)(tv.tv_sec * 1000000 + tv.tv_usec);
}


sg_etp_t *sg_etp_open(const char *server_addr, int server_port,
    sg_etp_on_open_func_t on_open,
    sg_etp_on_recv_func_t on_recv,
    sg_etp_on_sent_func_t on_sent,
    sg_etp_on_close_func_t on_close)
{
    struct etp_client_shared_real *client = NULL;
    struct sockaddr_in addr;
    int ret = -1;

    /* convert string address to sockaddr_in */
    ret = uv_ip4_addr(server_addr, server_port, &addr);
    if (ret <= 0) {
        sg_log_err("uv get address failed\n");
        return NULL;
    }

    /* new client */
    client = etp_client_shared_alloc();
    if (!client) {
        sg_log_err("etp client open error\n");
        return NULL;
    }
    etp_client_shared_set_callback(client, on_open, on_recv, on_sent, on_close);
    etp_client_shared_start(client, __make_kcp_conv_id(), (struct sockaddr *)&addr, NULL, NULL, interval_ms, client);
    sg_log_inf("open client with id %lu", client->conv);

    return client;
}

int sg_etp_run(sg_etp_t *client, int interval_ms)
{
    struct sg_etp_real *c = (struct sg_etp_real *)client;
    sg_assert(c);

    /* enter loop */
    uv_run(c->loop, UV_RUN_DEFAULT);

    return SG_OK;
}

sg_err_t sg_etp_send(sg_etp_t *client, const void *data, uint64_t size)
{
    return etp_client_shared_send(client, data, size);
}

/* current etp has speed question, delete this for temporary */
void sg_etp_set_max_send_speed(sg_etp_t *client, size_t kbps)
{
    etp_client_shared_set_max_send_speed(client, kbps);
}

void sg_etp_close(sg_etp_t *client)
{
    etp_client_shared_close(client);
}
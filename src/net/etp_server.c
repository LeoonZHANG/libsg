/**
 * etp_server.c
 * This is the server routin for kcp protocol with libuv, using UDP for
 * the data transfer protocol.
 */

#include <stdio.h>
#include <string.h>
#include "uv.h"
#include <sg/sg.h>
#include "../../3rdparty/kcp/ikcp.h"
#include "linkhash.h"
#include "uv_comm.h"
#include "etp_client_shared.h"
#include <sg/net/etp.h>
#include <sg/net/etp_server.h>
#include <sg/net/etp_private.h>

struct sg_etp_server_real {
    uv_loop_t                     loop;          /* uv loop handler, for kcp running in */
    uv_udp_t                      udp;           /* libuv udp handler, for kcp bearing protocol */
    struct sockaddr               addr;          /* address of server bind to */
    int                           backlog;       /* backlog flag */
    int                           max_conn;      /* maxium number of accepted client */
    int                           interval;      /* update kcp interval time */
    struct lh_table               *clients_list; /* clients list container */
    bool                          delay_close;   /* flag for delay close */
    sg_etp_server_on_open_func_t  on_open;       /* callback */
    sg_etp_server_on_recv_func_t  on_recv;       /* callback */
    sg_etp_server_on_sent_func_t  on_sent;       /* callback */
    sg_etp_server_on_error_func_t on_error;      /* callback */
    sg_etp_server_on_close_func_t on_close;      /* callback */
} sg_etp_server_t;

/****************************************************
 * callbacks
 ****************************************************/

/* these callbacks are for client */
static void __on_open(sg_etp_t *client)
{
    struct sg_etp_server_real *server = client->user_data;

    server->on_open(client);
}

static void __on_recv(sg_etp_t *client, char *data, size_t size)
{
    struct sg_etp_server_real *server = client->user_data;

    server->on_recv(client, data, size);
}

static void __on_sent(sg_etp_t *client, int status/*0:OK*/, void *data, size_t len)
{
    struct sg_etp_server_real *server = client->user_data;

    server->on_sent(client, status, data, len);
}

static void __on_close(sg_etp_t *client, int code, const char *reason)
{
    struct sg_etp_server_real *server = client->user_data;

    server->on_close(client, code, reason);

    lh_table_delete(server->clients_list, (void *)client->conv);

    if (!server->clients_list->head) { /* empty */
        if (server->delay_close) { /* close server for marked */
            uv_close((uv_handle_t*)&(server->udp), uv_common_on_close_done);

            uv_loop_close(&(server->loop));

            lh_table_free(server->clients_list);

            /*free(server->recv_data);*/
            free(server);
        }
    }
}

static void __on_error(sg_etp_t *client, const char *msg)
{
    struct sg_etp_server_real *server = client->user_data;

    server->on_error(client, msg);
}

static void __on_uv_recv_udp(uv_udp_t *handle, ssize_t nread,
        const uv_buf_t *rcvbuf, const struct sockaddr *addr, unsigned flags)
{
    IUINT32 conv = 0;
    int ret = 0;
    struct sg_etp_server_real *server = handle->data;
    etp_client_shared_t *client = NULL;

    /*sg_log_dbg("recv udp %d\n", nread);*/

    do {
        if (nread <= 0)
            break;

        /* get conversion id made by client side */
        ret = ikcp_get_conv(rcvbuf->base, (long)nread, (IUINT32 *)&conv);
        SG_ASSERT(ret == 1);

        /* check is existing client */
        client = (etp_client_shared_t *)lh_table_lookup(server->clients_list, (const void *)conv);

        /* new client */
        if (!client) {
            if (server->clients_list->count >= server->max_conn) {
                sg_log_inf("meet max connection %d, ignore", server->max_conn);
                break;
            }
            client = etp_client_shared_alloc();
            SG_ASSERT(client); /* create session must be successful */
            etp_client_shared_set_callbacks(client, __on_open, __on_recv, __on_sent, __on_close);
            lh_table_insert(server->clients_list, (void *)conv, client);
            etp_client_shared_start(client, conv, addr, &server->loop, server->udp, server->interval, (void *)server);
            sg_log_inf("client %lu connected", conv);
        }

        /* existing cliet */
        etp_client_shared_recv_udp(client, rcvbuf->base, nread);
    } while (0);

    free(rcvbuf->base);
}

/****************************************************
 * APIs
 ****************************************************/

sg_etp_server_t *
sg_etp_server_open(const char *server_addr, int server_port, int max_backlog,
        sg_etp_server_on_open_func_t    on_open,
        sg_etp_server_on_recv_func_t    on_recv,
        sg_etp_server_on_sent_func_t    on_sent,
        sg_etp_server_on_error_func_t   on_error,
        sg_etp_server_on_close_func_t   on_close)
{
    struct sg_etp_server_real *server = NULL;
    struct sockaddr_in addr;
    int ret = 0;

    /* create the client object */
    server = (struct sg_etp_server_real *)malloc(sizeof(struct sg_etp_server_real));
    SG_ASSERT_MALLOC(server);
    memset(server, 0, sizeof(struct sg_etp_server_real));

    server->backlog  = 0;
    server->max_conn = max_backlog;
    server->on_open  = on_open;
    server->on_recv  = on_recv;
    server->on_sent  = on_sent;
    server->on_error = on_error;
    server->on_close = on_close;

    /* create linkhash to store clients connected */
    server->clients_list = lh_kptr_table_new(server->max_conn, "etp server", NULL);

    /* get address */
    ret = uv_ip4_addr(server_addr, server_port, &addr);
    if (ret < 0) {
        sg_log_err("uv get address failed\n");
        goto error;
    }
    memcpy(&(server->addr), &addr, sizeof(struct sockaddr));

    /* initiate a new loop instead default loop */
    uv_loop_init(&(server->loop));

    /* init udp */
    ret = uv_udp_init(&(server->loop), &(server->udp));
    SG_ASSERT_MSG(ret >= 0, "init udp failed");
    s->udp.data = server;
    if (uv_udp_bind(&(server->udp), &(server->addr), 0) < 0) {
        sg_log_err("bind udp at port %d failed", server_port);
        goto error;
    }
    ret = uv_udp_recv_start(&(server->udp), on_uv_alloc_buffer, __on_uv_recv_udp);
    SG_ASSERT_MSG(ret >= 0, "start udp recv failed");

    return server;

error:
    if (server->clients_list)
        lh_table_free(server->clients_list);
    if (server)
        free(server);
    return NULL;
}

sg_err_t sg_etp_server_send(sg_etp_client_t *client, void *data, size_t size)
{
    return etp_client_shared_send(client, data, size);
}

void sg_etp_server_close_client(sg_etp_client_t *client)
{
    etp_client_shared_close(client);
}

sg_err_t sg_etp_server_get_client_addr(sg_etp_client_t *client, sg_vsstr_t *addr_out)
{
    char addr_tmp[256] = {0};

    uv_ip4_name((const struct sockaddr_in*)&(client->addr), addr, 256);

    sg_vsstrcpy(addr_out, addr_tmp);

    return SG_OK;
}

void sg_etp_server_run(sg_etp_server_t *server, int interval_ms)
{
    int ret = 0;
    struct sg_etp_server_real *s = (struct sg_etp_server_real *)server;

    SG_ASSERT_MSG(s, "server pointer is NULL");

    s->interval = interval_ms;



    /* enter loop */
    uv_run(&(s->loop), UV_RUN_DEFAULT);
}

void sg_etp_server_close(sg_etp_server_t *server)
{
    struct lh_entry *to_del = NULL;
    struct lh_entry *entry = NULL;
    struct sg_etp_server_real *s = (struct sg_etp_server_real *)server;

    /* traverse session list to close all sessions connected */
    entry = s->clients_list->head;
    while (NULL != entry) {
        to_del = entry;
        entry = entry->next;

        sg_etp_server_close_client((sg_etp_client_t *)to_del->v);

        s->delay_close = true;
    }

    /* there is no session to close */
    if (!s->delay_close) {
        uv_close((uv_handle_t*)&(s->udp), uv_common_on_close_done);

        uv_loop_close(&(s->loop));

        lh_table_free(s->clients_list);

        /*free(s->recv_data);*/
        free(s);
    }
}
/*===========================================================================*\
 * Vincent (vin@misday.com)
 *
 * This is the server routin for kcp protocol with libuv, using UDP for
 * the data transfore protocol.
 *
\*===========================================================================*/


/*===========================================================================*\
 * Header Files
\*===========================================================================*/
#include <stdio.h>
#include <string.h>
#include "uv.h"
#include "ikcp.h"
#include "linkhash.h"
#include <sg/net/etp.h>
#include <sg/net/etp_server.h>
#include <sg/net/etp_private.h>

/*===========================================================================*\
 * #define MACROS
\*===========================================================================*/



/*
 * 网速统计方法说明
 * 以SG_ETP_CALC_SPEED_INTERVAL_MS为最小统计单位，在这期间内计算一次瞬间网速，
 * 然后，存入大小为SG_ETP_SPEED_STAT_SAMPLE_COUNT的环形数组，
 * 最后，计算环形数组的所有元素值的平均值，作为当前的瞬间速度，这样的统计值比较平滑。
 */

/* 速度统计采样总数，必须是2的N次方 */
#define SG_ETP_SPEED_STAT_SAMPLE_COUNT 16

/* 多长统计一次瞬间发送速度 */
#define SG_ETP_CALC_SPEED_INTERVAL_MS 2000

/*===========================================================================*\
 * Local Type Declarations
\*===========================================================================*/


typedef struct sg_etp_server_real
{
    uv_loop_t                           loop;           /*< uv loop handler, for kcp running in */
    uv_udp_t                            udp;            /*< libuv udp handler, for kcp bearing protocol */

    struct sockaddr                     addr;           /*< address of server bind to */
    int                                 backlog;        /*< backlog flag */
    int                                 max_conn;       /*< maxium number of accepted client */
    int                                 interval;       /*< update kcp interval time */

    sg_etp_server_on_open_func_t        on_open;        /*< callback */
    sg_etp_server_on_data_func_t        on_data;        /*< callback */
    sg_etp_server_on_sent_func_t        on_sent;        /*< callback */
    sg_etp_server_on_error_func_t       on_error;       /*< callback */
    sg_etp_server_on_close_func_t       on_close;       /*< callback */

    struct lh_table*                    sessions;       /*< clients list container */

    bool_t                              to_close;       /*< flag for delay close */

    void                              * data;
}sg_etp_server_t;

/*===========================================================================*\
 * Exported Const Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Function Prototypes
\*===========================================================================*/

static void on_server_recv_udp(uv_udp_t* handle,
    ssize_t nread,
    const uv_buf_t* rcvbuf,
    const struct sockaddr* addr,
    unsigned flags);

static void s_on_open(sg_etp_t * client);
static void s_on_data(sg_etp_t * client, char *data, size_t size);
static void s_on_sent(sg_etp_t * client, int status/*0:OK*/, void * data, size_t len);
static void s_on_close(sg_etp_t * client, int code, const char *reason);
static void s_on_error(sg_etp_t * client, const char *msg);


/*===========================================================================*\
 * Local Inline Function Definitions and Function-Like Macros
\*===========================================================================*/

/*===========================================================================*\
 * Function Definitions
\*===========================================================================*/

static void on_server_recv_udp(uv_udp_t* handle,
    ssize_t nread,
    const uv_buf_t* rcvbuf,
    const struct sockaddr* addr,
    unsigned flags)
{
    IUINT32 conv = 0;
    int ret = 0;
    sg_etp_server_t * server = handle->data;
    sg_etp_session_t * session = NULL;

    /*LOG_D("recv udp %d\n", nread);*/

    do
    {
        /*SG_ASSERT_BRK(nread > 0, "no data read");*/
        if (nread <= 0) break;

        ret = ikcp_get_conv(rcvbuf->base, (long)nread, (IUINT32 *)&conv);
        SG_ASSERT_BRK(1 == ret, "get conv by ikcp failed");

        /* find client */
        session = (sg_etp_session_t *)lh_table_lookup(server->sessions, (const void *)conv);
        if (NULL == session)
        {
            if (server->sessions->count >= server->max_conn)
            {
                LOG_I("meet max connection %d, ignore", server->max_conn);
                break;
            }

            session = sg_etp_session_open(conv, addr, &(server->loop), server);;
            SG_ASSERT_BRK(NULL != session, "create session failed");

            sg_etp_session_set_callback(session,
                s_on_open, s_on_data, s_on_sent, s_on_close);

            /* link client */
            lh_table_insert(server->sessions, (void *)conv, session);

            sg_etp_session_start(session, server->interval, &(server->udp));

            s_on_open(session);

            LOG_I("client %lu connected", conv);
        }

        sg_etp_session_recv(session, rcvbuf->base, nread);
    }
    while (0);

    free(rcvbuf->base);
}


int sg_etp_server_init(void)
{
    return 0;
}

sg_etp_server_t *sg_etp_server_open(
    const char *server_addr, int server_port,
    int                             max_backlog,
    sg_etp_server_on_open_func_t    on_open,
    sg_etp_server_on_data_func_t    on_data,
    sg_etp_server_on_sent_func_t    on_sent,
    sg_etp_server_on_error_func_t   on_error,
    sg_etp_server_on_close_func_t   on_close
)
{
    sg_etp_server_t * server = NULL;
    struct sockaddr_in addr;
    int ret = 0;

    do
    {
        /* create the client object */
        server = (sg_etp_server_t *)malloc(sizeof(sg_etp_server_t));
        SG_ASSERT_BRK(NULL != server, "create sg_etp_server_t");

        memset(server, 0, sizeof(sg_etp_server_t));

        server->backlog     = 0;
        server->max_conn    = max_backlog;
        server->on_open     = on_open;
        server->on_data     = on_data;
        server->on_sent     = on_sent;
        server->on_error    = on_error;
        server->on_close    = on_close;

        /* create linkhash to store sessions connected */
        server->sessions = lh_kptr_table_new(server->max_conn, "etp server", NULL);

        /* get address */
        ret = uv_ip4_addr(server_addr, server_port, &addr);
        SG_ASSERT_BRK(ret >= 0, "get address failed");
        memcpy(&(server->addr), &addr, sizeof(struct sockaddr));

        /* initiate a new loop instead default loop */
        uv_loop_init(&(server->loop));

        return server;
    }
    while (0);

    if (NULL != server)
    {
        free(server);
        server = NULL;
    }

    return server;
}

int sg_etp_server_send(sg_etp_client_t * client, void *data, size_t size)
{
    return sg_etp_session_send(client, data, size);
}

void sg_etp_server_close_client(sg_etp_client_t * client)
{
    sg_etp_session_close(client);
}

char * sg_etp_server_get_client_addr(sg_etp_client_t * client)
{
    return sg_etp_session_get_client_addr(client);
}

void sg_etp_server_run(sg_etp_server_t * server, int interval_ms)
{
    int ret = 0;

    SG_ASSERT(NULL != server, "server pointer is NULL");

    server->interval = interval_ms;

    /* init udp */
    ret = uv_udp_init(&(server->loop), &(server->udp));
    SG_ASSERT(ret >= 0, "init udp failed");
    server->udp.data = server;
    ret = uv_udp_bind(&(server->udp), &(server->addr), 0);
    SG_ASSERT(ret >= 0, "bind udp failed");
    ret = uv_udp_recv_start(&(server->udp), on_uv_alloc_buffer, on_server_recv_udp);
    SG_ASSERT(ret >= 0, "start udp recv failed");

    /* network run */
    uv_run(&(server->loop), UV_RUN_DEFAULT);
}

void sg_etp_server_close(sg_etp_server_t * server)
{
    struct lh_entry * to_del = NULL;
    struct lh_entry * entry = NULL;

    /* traverse session list to close all sessions connected */
    entry = server->sessions->head;
    while (NULL != entry)
    {
        to_del = entry;
        entry = entry->next;

        sg_etp_server_close_client((sg_etp_client_t *)to_del->v);

        server->to_close = true;
    }

    /* there is no session to close */
    if (!server->to_close)
    {
        uv_close((uv_handle_t*)&(server->udp), on_uv_close_done);

        uv_loop_close(&(server->loop));

        lh_table_free(server->sessions);

        /*free(server->recv_data);*/
        free(server);
    }
}

void sg_etp_server_free(void)
{
}

/* these callbacks are for session */
static void s_on_open(sg_etp_t * client)
{
    sg_etp_server_t * server = sg_etp_session_get_data(client);

    SG_CALLBACK(server->on_open, client);

    /* TODO: other process */
}

static void s_on_data(sg_etp_t * client, char *data, size_t size)
{
    sg_etp_server_t * server = sg_etp_session_get_data(client);

    SG_CALLBACK(server->on_data, client, data, size);

    /* TODO: other process */
}

static void s_on_sent(sg_etp_t * client, int status/*0:OK*/, void * data, size_t len)
{
    sg_etp_server_t * server = sg_etp_session_get_data(client);

    SG_CALLBACK(server->on_sent, client, status, data, len);

    /* TODO: other process */
}

static void s_on_close(sg_etp_t * client, int code, const char *reason)
{
    sg_etp_server_t * server = (sg_etp_server_t * )sg_etp_session_get_data(client);

    SG_CALLBACK(server->on_close, client, code, reason);

    lh_table_delete(server->sessions, (void *)sg_etp_session_get_conv(client));

    if (NULL == server->sessions->head) /* empty */
    {
        if (server->to_close) /* close server for marked */
        {
            uv_close((uv_handle_t*)&(server->udp), on_uv_close_done);

            uv_loop_close(&(server->loop));

            lh_table_free(server->sessions);

            /*free(server->recv_data);*/
            free(server);
        }
    }
}

static void s_on_error(sg_etp_t * client, const char *msg)
{
    sg_etp_server_t * server = sg_etp_session_get_data(client);

    SG_CALLBACK(server->on_error, client, msg);

    /* TODO: other process */
}


/*============================================================================*/



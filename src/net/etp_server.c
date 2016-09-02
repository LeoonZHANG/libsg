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
#include "etp_server.h"

/*===========================================================================*\
 * #define MACROS
\*===========================================================================*/

typedef unsigned char bool;
#define true    1
#define false   0

#define OK      0
#define ERROR (-1)

#if defined(PLATFORM_WINDOWS)
/* FIXME: change variable argument macros definition if needed. */
#define LOG(fmt, ...)  printf("%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_E(prmpt, ...) LOG("E: " prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG("W: " prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG("I: " prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG("D: " prmpt, ##__VA_ARGS__)
#elif defined(PLATFORM_LINUX)
#define LOG(fmt, ...)  printf("%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_E(prmpt, ...) LOG("E: " prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG("W: " prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG("I: " prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG("D: " prmpt, ##__VA_ARGS__)
#elif defined(PLATFORM_MACOS) || defined(PLATFORM_BSD)
/* FIXME: change variable argument macros definition if needed. */
#define LOG(fmt, ...)  printf("%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_E(prmpt, ...) LOG("E: " prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG("W: " prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG("I: " prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG("D: " prmpt, ##__VA_ARGS__)
#else
/* FIXME: change variable argument macros definition if needed. */
#define LOG(fmt, ...)  printf("%s:%d " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_E(prmpt, ...) LOG("E: " prmpt, ##__VA_ARGS__)
#define LOG_W(prmpt, ...) LOG("W: " prmpt, ##__VA_ARGS__)
#define LOG_I(prmpt, ...) LOG("I: " prmpt, ##__VA_ARGS__)
#define LOG_D(prmpt, ...) LOG("D: " prmpt, ##__VA_ARGS__)
#endif

#define SG_ASSERT(exp, prmpt)          if (exp) {} else { LOG_W(prmpt); return; }
#define SG_ASSERT_RET(exp, prmpt, ret) if (exp) {} else { LOG_W(prmpt); return(ret); }
#define SG_ASSERT_BRK(exp, prmpt)      if (exp) {} else { LOG_W(prmpt); break; }

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


/* 速度统计采样总数，必须是2的N次方 */
#define SG_ETP_SPEED_STAT_SAMPLE_COUNT 16
/* 多长统计一次发送速度 */
#define SG_ETPSG_ETP_CALC_SPEED_INTERVAL_MS_MS 2000

typedef struct sg_etp_client_real
{
    IUINT32             conv;
    uv_loop_t         * loop;
    uv_udp_t          * udp;
    uv_timer_t          timer;
    uv_idle_t           idle;
    struct sockaddr     addr;
    ikcpcb            * kcp;
    IUINT32             kcp_update_time;
    int                 to_close;
    sg_etp_server_t   * server;
    sg_etp_server_on_open_func_t    on_open;
    sg_etp_server_on_data_func_t    on_data;
    sg_etp_server_on_close_func_t   on_close;
    void              * data;

    /* 统计速率相关 */
    uint64_t            last_time; /* 上一次统计时间 */
    int                 last_send_byte;
    int                 max_speed_limit;
    int                 head, tail;
    int                 last_speed[SG_ETP_SPEED_STAT_SAMPLE_COUNT];
    double              current_speed;
}sg_etp_client_t;

typedef struct sg_etp_server_real
{
    uv_loop_t                           loop_kcp;       /*< uv loop handler, for kcp running in, should not use this directly */
    uv_loop_t                         * loop;           /*< uv loop handler, for kcp running in */
    uv_udp_t                            udp;            /*< libuv udp handler, for kcp bearing protocol */
    uv_timer_t                          timer;          /*< libuv timer handler, for update kcp */
    uv_idle_t                           idle;           /*< libuv idle handler, for receive data */
    struct sockaddr                     addr;           /*< address of server bind to */
    int                                 backlog;        /*< backlog flag */
    int                                 max_conn;       /*< maxium number of accepted client */
    int                                 interval;       /*< update kcp interval time */
    sg_etp_server_on_open_func_t        on_open;        /*< callback */
    sg_etp_server_on_data_func_t        on_data;        /*< callback */
    sg_etp_server_on_close_func_t       on_close;       /*< callback */
    struct lh_table*                    clients;        /*< clients list container */
    /*void                              * recv_data;*/      /*< for receive data callback */
    /*size_t                              recv_data_len;*/  /*< for receive data callback */

    void                              * data;
}sg_etp_server_t;

typedef struct send_req_s
{
    uv_udp_send_t   req;
    uv_buf_t        buf;
    sg_etp_client_t* client;
}send_req_t;

/* TODO: merge c/s */
/* for libuv uv_queue_work, to process the received data via callback. */
typedef struct recv_data_s
{
    uv_work_t req;
    sg_etp_client_t *    client;
    size_t    data_len;
    char      data[1];  /* keep this field the last one for dynamic length */
}recv_data_t;

/*===========================================================================*\
 * Exported Const Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Function Prototypes
\*===========================================================================*/

static void on_uv_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf);
static int on_kcp_output(const char *buf, int len, struct IKCPCB *kcp, void *user);
static void on_udp_send_done(uv_udp_send_t* req, int status);
static void on_server_recv_udp(uv_udp_t* handle,
    ssize_t nread,
    const uv_buf_t* rcvbuf,
    const struct sockaddr* addr,
    unsigned flags);
static void on_uv_timer_cb(uv_timer_t* handle);
static void on_uv_idle_cb(uv_idle_t* handle);
static void on_uv_close_done(uv_handle_t* handle);
static void sg_etp_client_update_speed(sg_etp_client_t* client, uint64_t now);

static void recv_data_check(sg_etp_client_t * client);
static void recv_data_proc(uv_work_t *req);
static void recv_data_proc_cleanup(uv_work_t *req, int status);



/*===========================================================================*\
 * Local Inline Function Definitions and Function-Like Macros
\*===========================================================================*/

/*===========================================================================*\
 * Function Definitions
\*===========================================================================*/

/* for libuv */
static void on_uv_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
    buf->len = (unsigned long)size;
    buf->base = malloc(sizeof(char) * size);
}

/* for kcp callback */
static int on_kcp_output(const char *buf, int len, struct IKCPCB *kcp, void *user)
{
    sg_etp_client_t * client = (sg_etp_client_t *)user;
    int ret = -1;
    send_req_t * req = NULL;

    /*LOG_D("udp send: %d\n", len);*/

    do
    {
        // 限速
        if (client->max_speed_limit > 0 && client->current_speed > client->max_speed_limit)
            return ret;
        req = (send_req_t *)malloc(sizeof(send_req_t));
        SG_ASSERT_BRK(NULL != req, "create send_req_t failed");

        memset(req, 0, sizeof(send_req_t));

        req->client = client;
        req->buf.base = malloc(sizeof(char) * len);
        SG_ASSERT_BRK(NULL != req->buf.base, "create buf failed");
        req->buf.len = len;

        memcpy(req->buf.base, buf, len);

        ret = uv_udp_send((uv_udp_send_t*)req, client->udp, &req->buf, 1, &client->addr, on_udp_send_done);
        if (ret < 0)
        {
            free(req->buf.base); /* TODO: ensure free */
            free(req); /* TODO: ensure free ? */
            return -1;
        }

        return ret;
    } while (0);

    if (NULL != req)
    {
        if (NULL != req->buf.base)
        {
            free(req->buf.base);
            req->buf.base = NULL;
        }

        free(req);
        req = NULL;
    }

    return ret;
}

static void on_udp_send_done(uv_udp_send_t* req, int status)
{
    send_req_t * send_req = (send_req_t *)req;
    send_req->client->last_send_byte += send_req->buf.len; /* 统计网速 */
    free(send_req->buf.base); /* TODO: ensure free*/
    free(send_req); /** TODO: ensure free */
}

static void on_server_recv_udp(uv_udp_t* handle,
    ssize_t nread,
    const uv_buf_t* rcvbuf,
    const struct sockaddr* addr,
    unsigned flags)
{
    IUINT32 conv = 0;
    int ret = 0;
    sg_etp_server_t * server = handle->data;
    sg_etp_client_t * client = NULL;

    /*LOG_D("recv udp %d\n", nread);*/

    do
    {
        /*SG_ASSERT_BRK(nread > 0, "no data read");*/
        if (nread <= 0) break;

        ret = ikcp_get_conv(rcvbuf->base, (long)nread, (IUINT32 *)&conv);
        SG_ASSERT_BRK(1 == ret, "get conv by ikcp failed");

        /* TODO: find client */
        client = lh_table_lookup(server->clients, (const void *)conv);
        if (NULL == client)
        {
            if (server->clients->count >= server->max_conn)
            {
                LOG_I("meet max connection %d, ignore", server->max_conn);
                break;
            }

            client = (sg_etp_client_t *)malloc(sizeof(sg_etp_client_t));
            SG_ASSERT_BRK(NULL != client, "create client failed");
            /* link client */
            lh_table_insert(server->clients, (void *)conv, client);
            client->conv        = conv;
            client->loop        = server->loop;
            client->udp         = &(server->udp);
            client->on_open     = server->on_open;
            client->on_data     = server->on_data;
            client->on_close    = server->on_close;
            client->server      = server;
            memcpy(&(client->addr), addr, sizeof(struct sockaddr));

            client->kcp = ikcp_create(conv, (void*)client);
            SG_ASSERT_BRK(client->kcp != NULL, "create ikcp failed");

            client->kcp->output = on_kcp_output;

            ret = ikcp_nodelay(client->kcp, 1, server->interval, 2, 1);

            LOG_I("conn from %lu\n", client->conv);

            client->on_open(client);
        }

        client->kcp_update_time = 0; /* clear next update */
        ikcp_input(client->kcp, rcvbuf->base, nread);
    }
    while (0);

    free(rcvbuf->base);
}

static void on_uv_timer_cb(uv_timer_t* handle)
{
    sg_etp_server_t * server = handle->data;
    sg_etp_client_t * client = NULL;
    sg_etp_client_t * to_del = NULL;
    struct lh_entry * entry  = NULL;
    IUINT32 now = (IUINT32)uv_now(server->loop);

    /*LOG_D("update %d\n", DBLL_Get_Count(&(server->clients)));*/

    /* traverse client list */
    entry = server->clients->head;
    while (NULL != entry)
    {
        client = entry->v;

        /*printf("update %d\n", client->conv);*/
        /* 更新网速 */
        sg_etp_client_update_speed(client, now);

        if (now >= client->kcp_update_time)
        {
            ikcp_update(client->kcp, now);
            client->kcp_update_time = ikcp_check(client->kcp, now);
        }

        recv_data_check(client);

        /* must get next before delete */
        entry = entry->next;

        if (client->to_close)
        {
            sg_etp_server_close_client(client);
        }
    }
}

#if 0
static void on_uv_idle_cb(uv_idle_t* handle)
{
    int ret = 0;
    int len = 0;
    sg_etp_server_t * server = handle->data;
    sg_etp_client_t * client = NULL;
    struct lh_entry * entry = NULL;

    /* traverse client list */
    entry = server->clients->head;
    while (NULL != entry)
    {
        client = entry->v;

        /* check available data */
        len = ikcp_peeksize(client->kcp);
        if (len > 0)
        {
            /* prepare memory */
            if (server->recv_data_len < len)
            {
                server->recv_data = realloc(server->recv_data, len);
                SG_ASSERT(NULL != server->recv_data, "realloc failed");
                server->recv_data_len = len;
            }

            /* receive data and call on_message callback */
            ret = ikcp_recv(client->kcp, server->recv_data, len);
            if (ret >= 0)
            {
                client->on_data(client, server->recv_data, ret);
            }
        }

        entry = entry->next;
    }
}

#endif

/* @TODO: merge c/s */
static void recv_data_check(sg_etp_client_t * client)
{
    int len = 0;
    recv_data_t * req = NULL;

    len = ikcp_peeksize(client->kcp);
    if (len > 0)
    {
        req = malloc(sizeof(recv_data_t) + len);
        if (NULL != req)
        {
            len = ikcp_recv(client->kcp, req->data, len);
            if (len >= 0)
            {
                req->data_len = len;
                req->client = client;
                uv_queue_work(client->loop, &req->req, recv_data_proc, recv_data_proc_cleanup);
            }
            else
            {
                free(req);
            }
        }
        else
        {
            LOG_E("recv_data malloc failed");
        }
    }
}

/* @TODO: merge c/s */
static void recv_data_proc(uv_work_t * req)
{
    recv_data_t * recv = (recv_data_t *)req;
    recv->client->on_data(recv->client, recv->data, recv->data_len);
}

/* @TODO: merge c/s */
static void recv_data_proc_cleanup(uv_work_t *req, int status)
{
    free(req);
}

/* callback after udp closed */
static void on_uv_close_done(uv_handle_t* handle)
{

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
        server->on_close    = on_close;

        server->clients = lh_kptr_table_new(server->max_conn, "etp server", NULL);

        /* get address */
        ret = uv_ip4_addr(server_addr, server_port, &addr);
        SG_ASSERT_BRK(ret >= 0, "get address failed");
        memcpy(&(server->addr), &addr, sizeof(struct sockaddr));

        server->loop = &(server->loop_kcp);
        uv_loop_init(server->loop); /* initiate a new loop instead default loop */

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

int sg_etp_server_send_data(sg_etp_client_t * client, void *data, size_t size)
{
    client->kcp_update_time = 0; /* clear next update */
    return ikcp_send(client->kcp, data, size);
}

void sg_etp_server_close_client(sg_etp_client_t * client)
{
    sg_etp_server_t * server = client->server;

    if (ikcp_waitsnd(client->kcp) > 0 || ikcp_peeksize(client->kcp) > 0)
    {
        client->to_close = true; /* mark for close later */
        return;
    }

    client->on_close(client, OK, "");

    lh_table_delete(server->clients, (void *)client->conv);
    ikcp_release(client->kcp);
    free(client);
}

char *sg_etp_server_get_client_addr(sg_etp_client_t * client)
{
    char * addr = NULL;

    addr = malloc(256);

    uv_ip4_name((const struct sockaddr_in*)&(client->addr), addr, 256);

    return addr;
}

void sg_etp_server_run(sg_etp_server_t * server, int interval_ms)
{
    int ret = 0;

    SG_ASSERT(NULL != server, "server pointer is NULL");

    server->interval = interval_ms;

    /* init udp */
    ret = uv_udp_init(server->loop, &(server->udp));
    SG_ASSERT(ret >= 0, "init udp failed");
    server->udp.data = server;
    ret = uv_udp_bind(&(server->udp), &(server->addr), 0);
    SG_ASSERT(ret >= 0, "bind udp failed");
    ret = uv_udp_recv_start(&(server->udp), on_uv_alloc_buffer, on_server_recv_udp);
    SG_ASSERT(ret >= 0, "start udp recv failed");

    /* start a timer for kcp update and receiving */
    ret = uv_timer_init(server->loop, &(server->timer));
    SG_ASSERT(ret >= 0, "init timer failed");
    server->timer.data = server; /* link client pointer to timer */
    ret = uv_timer_start(&(server->timer), on_uv_timer_cb, interval_ms, interval_ms);
    SG_ASSERT(ret >= 0, "start timer failed");

    /* reg idle for data process */
    #if 0
    ret = uv_idle_init(server->loop, &(server->idle));
    SG_ASSERT(ret >= 0, "init idle failed");
    server->idle.data = server;
    ret = uv_idle_start(&(server->idle), on_uv_idle_cb);
    SG_ASSERT(ret >= 0, "start idle failed");
    #endif

    /* network run */
    uv_run(server->loop, UV_RUN_DEFAULT);
}

void sg_etp_server_close(sg_etp_server_t * server)
{
    sg_etp_client_t * client = NULL;
    struct lh_entry * to_del = NULL;
    struct lh_entry * entry = NULL;

    /* traverse client list */
    entry = server->clients->head;
    while (NULL != entry)
    {
        to_del = entry;
        entry = entry->next;

        sg_etp_server_close_client((sg_etp_client_t *)to_del->v);
    }

    /* FIXME: should wait all client connection been closed. */

    uv_close((uv_handle_t*)&(server->udp), on_uv_close_done);

    uv_loop_close(server->loop);

    lh_table_free(server->clients);

    /*free(server->recv_data);*/
    free(server);
}

static void sg_etp_client_update_speed(sg_etp_client_t* client, uint64_t now)
{
    if (client->max_speed_limit <=0 )
        return;

    if (!client->last_time) {
        // start to record
        client->last_time = now;
        return;
    }
    else {
        // update the last_speed, every tow second
        uint64_t msecond = now - client->last_time;
        if (msecond > SG_ETP_CALC_SPEED_INTERVAL_MS) {
            // 平滑处理
            int count = 0;
            double speed = client->last_send_byte * 1.0 / msecond * 1000;

            // 加入队列
            client->last_speed[client->tail++] = speed;
            client->tail &= (SG_ETP_SPEED_STAT_SAMPLE_COUNT-1);
            if (client->tail == client->head)
                client->head = (client->head + 1)&(SG_ETP_SPEED_STAT_SAMPLE_COUNT-1);

            speed = 0;
            for (int i = client->head; i != client->tail; i=(i+1)&(SG_ETP_SPEED_STAT_SAMPLE_COUNT-1)) {
                speed += client->last_speed[i];
                count += 1;
            }
            speed /= count;
            client->current_speed = speed;
            printf("send %lf kib/s %d\n", speed/1024, count);
            client->last_time = now;
            client->last_send_byte = 0;
        }

    }
}

void sg_etp_server_set_max_send_speed(sg_etp_client_t *client, size_t kbps)
{
    client->max_speed_limit = kbps*1024;
    client->last_send_byte = 0;
    client->last_time = 0;
    if (!kbps){
        client->tail = client->head = 0;
        memset(client->last_speed, 0, sizeof(client->last_speed));
        client->current_speed = 0;
    }
}

void sg_etp_server_free(void)
{
}


/*============================================================================*/



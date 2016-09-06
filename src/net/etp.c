/*===========================================================================*\
 * Vincent (vin@misday.com)
 *
 * This is the client routin for kcp protocol with libuv, using UDP for
 * the data transfore protocol.
 *
\*===========================================================================*/

/*===========================================================================*\
 * Header Files
\*===========================================================================*/
#include "uv.h"
#include "ikcp.h"
#include "etp.h"
#include "etp_private.h"


/*===========================================================================*\
 * #define MACROS
\*===========================================================================*/

/**-----------------------------------------------------------------------------
 * Speed start
 */

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

void sg_etp_update_speed(sg_etp_t * client, uint64_t now);
void sg_etp_set_max_send_speed(sg_etp_t * client, size_t kbps);

/**
 * Speed end
 * -----------------------------------------------------------------------------
 */

/*===========================================================================*\
 * Local Type Declarations
\*===========================================================================*/
typedef struct sg_etp_real
{
    IUINT32                     conv;               /*< conversation id, for kcp */

    uv_loop_t *                 loop;               /*< uv loop handler, for kcp running in */
    uv_loop_t                   loop_hdl;           /*< uv loop handler, for kcp running in */
    uv_udp_t *                  udp;                /*< libuv udp handler, for kcp bearing protocol */
    uv_udp_t                    udp_hdl;            /*< libuv udp handler, for kcp bearing protocol */

    uv_idle_t                   idle;

    uv_timer_t                  timer;              /*< libuv timer handler, for update kcp */

    struct sockaddr             addr;               /*< address of the server */

    ikcpcb *                    kcp;                /*< kcp pointer */
    IUINT32                     kcp_update_time;    /*< next update time, for kcp, if =0 will updated in the next time period */
    uint32_t                    interval;           /*< interval of the libuv timer, for updating kcp_update() */

    IUINT32                     timeout;            /*< session timeout */
    IUINT32                     recv_data_time;     /*< received data timing */

    char *                      recv_buf;
    size_t                      recv_buf_len;

    bool_t                      to_close;           /*< flag for delay close */

    sg_etp_on_open_func_t       on_open;            /*< callback, */
    sg_etp_on_data_func_t       on_data;            /*< callback, */
    sg_etp_on_sent_func_t       on_sent;            /*< callback, */
    sg_etp_on_close_func_t      on_close;           /*< callback, */

    void *                      data;


    /* 统计速率相关,采用滑窗试计算 */
    uint64_t            last_time; /* 上一次统计时间 */
    int                 last_send_byte;
    int                 max_speed_limit;
    int                 head, tail;
    int                 last_speed[SG_ETP_SPEED_STAT_SAMPLE_COUNT];
    double              current_speed;
}sg_etp_t;




/*===========================================================================*\
 * Exported Const Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Function Prototypes
\*===========================================================================*/

static void on_client_recv_udp(
    uv_udp_t                * handle,
    ssize_t                   nread,
    const uv_buf_t          * rcvbuf,
    const struct sockaddr   * addr,
    unsigned int              flags);

/*===========================================================================*\
 * Local Inline Function Definitions and Function-Like Macros
\*===========================================================================*/

/*===========================================================================*\
 * Function Definitions
\*===========================================================================*/

/* for libuv */
static void on_client_recv_udp(uv_udp_t *handle, ssize_t nread,
        const uv_buf_t *rcvbuf, const struct sockaddr *addr, unsigned int flags)
{
    sg_etp_t * client = NULL;

    /*LOG_D("recv udp %d\n", nread);*/

    do
    {
        if (nread <= 0) { break; }

        client = (sg_etp_t *)(handle->data);

        sg_etp_session_recv(client, rcvbuf->base, nread);

    } while (0);

    free(rcvbuf->base);
}

int sg_etp_init()
{
    return 0;
}

sg_etp_t *sg_etp_open(const char *server_addr, int server_port,
    sg_etp_on_open_func_t on_open,
    sg_etp_on_data_func_t on_data,
    sg_etp_on_sent_func_t on_sent,
    sg_etp_on_close_func_t on_close)
{
    sg_etp_t * client = NULL;
    IUINT32 conv;
    struct timeval tv;
    struct sockaddr_in addr;
    int ret = -1;

    do
    {
        /* 原来client->conv=client，现在改成时间，因为反复关开此程序进程，client指针可能相同。 */
        gettimeofday(&tv, NULL);
        conv = (IUINT32)(tv.tv_sec * 1000000 + tv.tv_usec); /* TODO: alloc the conversation id */

        /* get address */
        ret = uv_ip4_addr(server_addr, server_port, &addr);
        SG_ASSERT_BRK(ret >= 0, "get address failed");

        client = sg_etp_session_open(conv, (struct sockaddr *)&addr, NULL, client);

        sg_etp_session_set_callback(client,
            on_open,
            on_data,
            on_sent,
            on_close);

        LOG_I("open client %lu", conv);

        return client;
    } while (0);

    if (NULL != client)
    {
        free(client);
        client = NULL;
    }

    return client;
}

int sg_etp_run(sg_etp_t *client, int interval_ms)
{
    int ret = ERROR;

    SG_ASSERT_RET(NULL != client, "client pointer is NULL", ret);

    sg_etp_session_start(client, interval_ms, NULL);

    /* enter loop */
    uv_run(client->loop, UV_RUN_DEFAULT);

    return OK;
}

int sg_etp_send(sg_etp_t * client, const void * data, uint64_t size)
{
    return sg_etp_session_send(client, data, size);
}

uint32_t sg_etp_now(sg_etp_t * client)
{
    return (uint32_t)uv_now(client->loop);
}

void sg_etp_close(sg_etp_t *client)
{
    sg_etp_session_close(client);
}

void sg_etp_free(void)
{

}

/******************************************************************************/
/* libuv udp callback for both client/server */
void on_uv_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf)
{
    buf->len = (unsigned long)size;
    buf->base = malloc(sizeof(char) * size);
}

/* libuv udp callback for both client/server */
void on_uv_close_done(uv_handle_t* handle)
{
}

/******************************************************************************/

void sg_etp_update_speed(sg_etp_t * client, uint64_t now)
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

void sg_etp_set_max_send_speed(sg_etp_t * client, size_t kbps)
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

/******************************************************************************/
/******************************************************************************
 * kcp session
 *****************************************************************************/

#define SG_ETP_SESSION_TIMEOUT (3 * 60 * 1000)

typedef struct send_req_s
{
    uv_udp_send_t   req;        /*< libuv send handler, keep this field the first one */
    uv_buf_t        buf;        /*< buf of data */
    void *          session;    /* send_done怎么知道这个数据是谁发，给谁发的？加上client，用于统计流量，限制发送速度时，要通过这个client关联。 */
}send_req_t;


/* TODO: merge c/s */
/* for libuv uv_queue_work, to process the received data via callback. */
typedef struct recv_req_s
{
    uv_work_t   req;        /*< libuv work handler, should keep this field the 1st one */
    void *      session;    /*< */
    size_t      data_len;   /*< */
    char        data[1];    /* keep this field the last one for dynamic length */
}recv_req_t;

static void on_uv_timer_cb(uv_timer_t * handle);
#if 0
static void recv_data_check(sg_etp_session_t * session);
static void recv_data_proc(uv_work_t * req);
static void recv_data_proc_cleanup(uv_work_t *req, int status);
#endif
static void on_uv_idle_cb(uv_idle_t * handle);
static void on_udp_send_done(uv_udp_send_t* req, int status);
static int on_kcp_output(const char *buf, int len, struct IKCPCB *kcp, void *user);


/* for libuv */
static void on_uv_timer_cb(uv_timer_t * handle)
{
    sg_etp_session_t * session = handle->data;
    IUINT32 now = 0;

    /*LOG_D("update %d", client->conv);*/

    /* update ikcp */
    now = (IUINT32)uv_now(session->loop);
    sg_etp_update_speed((sg_etp_t *)session, now);
    if (now >= session->kcp_update_time)
    {
        ikcp_update(session->kcp, now);
        session->kcp_update_time = ikcp_check(session->kcp, now);

        LOG_D("update %lu @ %lu, timeout: %lu", session->conv, session->kcp_update_time, session->recv_data_time);

        /* check received data and add to work queue */
        //recv_data_check(session);
        if (ikcp_peeksize(session->kcp) > 0)
        {
            uv_idle_start(&(session->idle), on_uv_idle_cb);
        }
    }

    /* check if session is timeout */
    if (session->recv_data_time < now)
    {
        session->to_close = true; /* mark to close this session. */
        ikcp_flush(session->kcp);
        LOG_I("session %lu timeout, will be closed", session->conv);
    }

    /* check if should close this session */
    if (session->to_close)
    {
        sg_etp_session_close(session);
    }
}

#if 0
/*  */
static void recv_data_check(sg_etp_session_t * session)
{
    int len = 0;
    recv_req_t * req = NULL;

    while ((len = ikcp_peeksize(session->kcp)) > 0)
    /*if (len > 0)*/
    {
        req = malloc(sizeof(recv_req_t) + len);
        if (NULL != req)
        {
            len = ikcp_recv(session->kcp, req->data, len);
            if (len >= 0)
            {
                req->data_len = len;
                req->session  = session;
                uv_queue_work(session->loop, &req->req, recv_data_proc, recv_data_proc_cleanup);
            }
            else
            {
                free(req);
            }
        }
        else
        {
            LOG_E("recv_data malloc failed");
            break;
        }
    }
}

/*  */
static void recv_data_proc(uv_work_t * req)
{
    recv_req_t * recv = (recv_req_t *)req;
    sg_etp_session_t * session = (sg_etp_session_t *)recv->session;

    SG_CALLBACK(session->on_data, session, recv->data, recv->data_len);
}

/* @TODO: merge c/s */
static void recv_data_proc_cleanup(uv_work_t *req, int status)
{
    free(req);
}
#endif

static void on_uv_idle_cb(uv_idle_t * handle)
{
    sg_etp_session_t * session = (sg_etp_session_t *)handle->data;
    int len = 0;

    len = ikcp_peeksize(session->kcp);
    if (len > 0)
    {
        if (len > session->recv_buf_len)
        {
            session->recv_buf = realloc(session->recv_buf, len);
            session->recv_buf_len = len;
        }

        SG_ASSERT(NULL != session->recv_buf, "alloc recv buf failed");
        
        len = ikcp_recv(session->kcp, session->recv_buf, len);
        SG_CALLBACK(session->on_data, session, session->recv_buf, len);
    }
    else
    {
        uv_idle_stop(&(session->idle));
    }
}

/* for libuv udp callback */
static void on_udp_send_done(uv_udp_send_t* req, int status)
{
    send_req_t * send_req = (send_req_t *)req;
    sg_etp_session_t * session = (sg_etp_session_t *)send_req->session;

    SG_CALLBACK(session->on_sent, session, status, send_req->buf.base, send_req->buf.len);

    session->last_send_byte += send_req->buf.len; /* ?????? */

    free(send_req->buf.base); /* ensure free*/
    free(send_req); /** ensure free */
}


/* for kcp callback */
static int on_kcp_output(const char *buf, int len, struct IKCPCB *kcp, void *user)
{
    sg_etp_t * session = (sg_etp_t *)user;
    int ret = -1;
    send_req_t * req = NULL;

    /*LOG_D("udp send: %d\n", len);*/

    do
    {
        /* 限速 */
        if (session->max_speed_limit > 0 && session->current_speed > session->max_speed_limit)
        {
            return ret;
        }

        req = (send_req_t *)malloc(sizeof(send_req_t));
        SG_ASSERT_BRK(NULL != req, "create send_req_t failed");

        memset(req, 0, sizeof(send_req_t));

        req->buf.base = malloc(sizeof(char) * len);
        SG_ASSERT_BRK(NULL != req->buf.base, "create buf failed");
        req->buf.len = len;

        req->session = session;   /* remember the client pointer to used in on_udp_send_done */

        memcpy(req->buf.base, buf, len);

        ret = uv_udp_send((uv_udp_send_t*)req, session->udp, &req->buf, 1, &session->addr, on_udp_send_done);
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






/******************************************************************************/

sg_etp_session_t * sg_etp_session_open(IUINT32 conv, const struct sockaddr * addr, uv_loop_t * loop, void * data)
{
    sg_etp_session_t * session = NULL;

    do
    {
        session = (sg_etp_session_t *)malloc(sizeof(sg_etp_session_t));
        SG_ASSERT_BRK(NULL != session, "create client failed");

        memset(session, 0, sizeof(sg_etp_session_t));

        session->data = data;
        session->conv = conv;

        memcpy(&(session->addr), addr, sizeof(struct sockaddr));

        if (NULL == loop) /* self-contained loop, for client */
        {
            session->loop = &(session->loop_hdl);
            uv_loop_init(session->loop);
        }
        else
        {
            session->loop = loop;
        }

        /* create the kcp object */
        session->kcp = ikcp_create(session->conv, (void*)session);
        SG_ASSERT_BRK(NULL != session->kcp, "create ikcp failed");

        session->kcp->output = on_kcp_output;

        session->timeout        = SG_ETP_SESSION_TIMEOUT;
        session->recv_data_time = uv_now(session->loop) + session->timeout;

        return session;
    } while (0);

    if (NULL != session)
    {
        free(session);
        session = NULL;
    }

    return session;
}

int sg_etp_session_set_callback(sg_etp_session_t * session,
    sg_etp_on_open_func_t on_open,
    sg_etp_on_data_func_t on_data,
    sg_etp_on_sent_func_t on_sent,
    sg_etp_on_close_func_t on_close
)
{
    session->on_open = on_open;
    session->on_data = on_data;
    session->on_sent = on_sent;
    session->on_close = on_close;

    return OK;
}

int sg_etp_session_close(sg_etp_session_t * session)
{
    LOG_D("ikcp_peeksize: %d, ikcp_waitsnd: %d",
        ikcp_peeksize(session->kcp), ikcp_waitsnd(session->kcp));

    if (ikcp_waitsnd(session->kcp) > 0 || ikcp_peeksize(session->kcp) > 0)
    {
        session->to_close = true; /* mark for close later */
        return OK;
    }

    LOG_D("close session");

    ikcp_release(session->kcp);

    uv_timer_stop(&(session->timer));

    if (session->udp == &(session->udp_hdl)) /* self-contained udp, should close here */
    {
        uv_close((uv_handle_t*)session->udp, on_uv_close_done);
    }

    if (session->loop == &(session->loop_hdl)) /* self-contained loop, should close here */
    {
        uv_loop_close(session->loop);
    }

    SG_CALLBACK(session->on_close, session, OK, "ok");

    if (NULL != session->recv_buf)
    {
        free(session->recv_buf);
    }

    free(session);

    return OK;
}

int sg_etp_session_start(sg_etp_session_t * session, int interval_ms, uv_udp_t * udp)
{
    int ret = ERROR;

    SG_ASSERT_RET(NULL != session, "session pointer is NULL", ret);

    session->interval = interval_ms;

    if (NULL == udp) /* self-contained udp, for client */
    {
        /* init udp */
        session->udp = &(session->udp_hdl);
        ret = uv_udp_init(session->loop, session->udp);
        SG_ASSERT_RET(ret >= 0, "init udp failed", ERROR);
        session->udp->data = session;
        ret = uv_udp_recv_start(session->udp, on_uv_alloc_buffer, on_client_recv_udp);
        SG_ASSERT_RET(ret >= 0, "start udp recv failed", ERROR);
    }
    else
    {
        session->udp = udp;
    }

    ret = ikcp_nodelay(session->kcp, 1, interval_ms, 2, 0);
    SG_ASSERT_RET(ret >= 0, "ikcp nodelay failed", ERROR);

    /* start a timer for kcp update and receiving */
    ret = uv_timer_init(session->loop, &(session->timer));
    SG_ASSERT_RET(ret >= 0, "init timer failed", ERROR);
    session->timer.data = session; /* link client pointer to timer */
    ret = uv_timer_start(&(session->timer), on_uv_timer_cb, session->interval, session->interval);
    SG_ASSERT_RET(ret >= 0, "start timer failed", ERROR);

    uv_idle_init(session->loop, &(session->idle));
    session->idle.data = session;

    SG_CALLBACK(session->on_open, session);

    return OK;
}

int sg_etp_session_send(sg_etp_session_t * session, const void * data, size_t size)
{
    session->kcp_update_time = 0; /* clear to update kcp in next update period */
    return ikcp_send(session->kcp, data, size);
}

int sg_etp_session_recv(sg_etp_session_t * session, void * data, size_t size)
{
    uint64_t now = uv_now(session->loop);

    session->recv_data_time = now + session->timeout;

    session->kcp_update_time = 0; /* clear to update kcp in next update period */
    return ikcp_input(session->kcp, data, (long)size);
}

void * sg_etp_session_get_data(sg_etp_session_t * session)
{
    return session->data;
}

char * sg_etp_session_get_client_addr(sg_etp_session_t * session)
{
    char * addr = NULL;

    addr = malloc(256);

    uv_ip4_name((const struct sockaddr_in*)&(session->addr), addr, 256);

    return addr;
}

IUINT32 sg_etp_session_get_conv(sg_etp_session_t * session)
{
    return session->conv;
}

void sg_etp_session_set_timeout(sg_etp_session_t * session, uint64_t timeout)
{
    session->timeout = timeout;
}




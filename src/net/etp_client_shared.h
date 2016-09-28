/**
 * etp_client_shared.h
 * Etp client shared code for etp.c and etp_server.c.
 */

#ifndef LIBSG_ETP_CLIENT_SHARED_H
#define LIBSG_ETP_CLIENT_SHARED_H

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sg/sg.h>
#include <sg/math/speed.h>
#include "../../3rdparty/kcp/ikcp.h"

#if defined(SG_OS_WINDOWS)
# include <winsock2.h>
# include <windows.h>
# pragma comment(lib ,"ws2_32.lib")
# pragma comment(lib, "psapi.lib")
# pragma comment(lib, "Iphlpapi.lib")
# pragma comment(lib, "userenv.lib")
#elif defined(SG_OS_LINUX)
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <sys/epoll.h>
# include <sys/time.h>
#elif defined(SG_OS_MACOS)
# include <sys/event.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 3 minutes timeout duration for etp connection */
#define SG_ETP_SESSION_TIMEOUT (3 * 60 * 1000)

typedef struct etp_client_shared_real {
    /* these parameters will be initialized
       in different way for etp.c and etp_server.c */
    IUINT32         conv;            /* conversation id, for kcp */
    uv_loop_t       *loop;           /* uv loop handler, for kcp running in */
    bool            alloc_loop;
    uv_udp_t        *udp;            /* libuv udp handler, for kcp bearing protocol */
    bool            alloc_udp;
    struct sockaddr addr;            /* address of client */

    /* these parameters will be initialized
       in same way for etp.c and etp_server.c */
    uv_idle_t       idler;                /* libuv idle monitor */
    uv_timer_t      timer;                /* libuv timer handler, for update kcp */
    ikcpcb          *kcp;                 /* kcp pointer */
    IUINT32         kcp_next_update_time; /* next update time for kcp provided by ikcp_check(...), if =0 will updated in the next default timer period */
    uint32_t        uv_timer_interval_ms; /* interval of the libuv timer, timer is for updating kcp_update() */
    IUINT32         conn_timeout_ms;      /* etp connection timeout duration in milliseconds */
    IUINT32         recv_timeout_timing;  /* data receive timeout timing, it's a unix time */
    char            *recv_buf;            /* receive data swap buffer for temporary */
    size_t          recv_buf_len;         /* recv_buf space size */
    bool            delay_close;          /* delay close flag */
    int             max_limit_speed;      /* max speed allowed in Kb/s */
    sg_speed_counter_t     *recv_speed_counter; /* receive speed stat tool */
    sg_speed_counter_t     *send_speed_counter; /* send speed stat tool */
    sg_etp_on_open_func_t  on_open;       /* callback */
    sg_etp_on_recv_func_t  on_recv;       /* callback */
    sg_etp_on_sent_func_t  on_sent;       /* callback */
    sg_etp_on_close_func_t on_close;      /* callback */
    void                   *user_data;    /* context */
} etp_client_shared_t;

typedef struct send_req {
    uv_udp_send_t   req;        /* libuv send handler, keep this field the first one */
    uv_buf_t        buf;        /* buf of data */
    void            *session;   /* send_done怎么知道这个数据是谁发，给谁发的？加上client，用于统计流量，限制发送速度时，要通过这个client关联。 */
} send_req_t;


/* on libuv receive event callback */
static void __on_uv_recv_udp(uv_udp_t *handle, ssize_t nread,
                             const uv_buf_t *rcvbuf, const struct sockaddr *addr, unsigned int flags)
{
    etp_client_shared_t *client = (etp_client_shared_t *)(handle->data);

    /*sg_log_dbg("recv udp %d\n", nread);*/

    if (nread > 0)
        etp_client_shared_recv_udp(client, rcvbuf->base, nread);

    free(rcvbuf->base);
}

/* using idle period to receive data or something else, avoid to lock shared resources */
static void __on_uv_idle(uv_idle_t *idler)
{
    etp_client_shared_t *self = (etp_client_shared_t *)idler->data;
    int len = 0;

    len = ikcp_peeksize(self->kcp);/* check the size of next message in the recv queue */
    if (len > 0) {
        if (len > self->recv_buf_len) {
            self->recv_buf = realloc(self->recv_buf, len);
            self->recv_buf_len = len;
        }

        SG_ASSERT_MALLOC(self->recv_buf);

        len = ikcp_recv(self->kcp, self->recv_buf, len);
        self->on_recv(self, self->recv_buf, len);
    } else /* recv queue is empty, means no data need to be received */
        uv_idle_stop(&(self->idler)); /* stop watching idle and wait to enter next loop */
}

static void __on_uv_timer(uv_timer_t *timer)
{
    etp_client_shared_t *self = timer->data;
    IUINT32 now = 0;

    /*sg_log_dbg("update %d", client->conv);*/

    /* update ikcp */
    now = (IUINT32)uv_now(self->loop);
    /* TODO: update speed counter here */
    if (now >= self->kcp_next_update_time) {
        ikcp_update(self->kcp, now);
        self->kcp_next_update_time = ikcp_check(self->kcp, now);

        sg_log_dbg("update %lu @ %lu, timeout: %lu", self->conv, self->kcp_next_update_time, self->recv_timeout_timing);

        if (ikcp_peeksize(self->kcp) > 0) /* if kcp recv data */
            uv_idle_start(&(self->idler), __on_uv_idle); /* set idle monitor callback and start to watch */
    }

    /* check if session is timeout */
    if (self->recv_timeout_timing < now) {
        self->delay_close = true; /* mark to close this session. */
        ikcp_flush(self->kcp);
        sg_log_inf("client %lu timeout, will be closed", self->conv);
    }

    /* check if should close this session */
    if (self->delay_close)
        etp_client_shared_close(self);
}

static void __on_uv_send_done(uv_udp_send_t *req, int status)
{
    send_req_t *send_req = (send_req_t *)req;
    etp_client_shared_t *self = (etp_client_shared_t *)send_req->session; /* session? */

    sg_speed_counter_reg(self->send_speed_counter, send_req->buf.len);

    self->on_sent(self, status, send_req->buf.base, send_req->buf.len);

    free(send_req->buf.base);
    free(send_req);
}

/* for kcp callback to send any udp data, including payload or kcp commands */
static int __on_kcp_output(const char *buf, int len, struct IKCPCB *kcp, void *user)
{
    etp_client_shared_t *self = (etp_client_shared_t *)user;
    int ret = -1;
    send_req_t *req = NULL;

    /*sg_log_dbg("udp send: %d\n", len);*/

    /* TODO: limit the speed */
    /*if (self->max_limit_speed > 0 && self->current_speed > self->max_limit_speed)
        return ret;*/

    req = (send_req_t *)malloc(sizeof(send_req_t));
    SG_ASSERT(req != NULL);

    memset(req, 0, sizeof(send_req_t));

    req->buf.base = malloc(sizeof(char) * len);
    SG_ASSERT_MALLOC(req->buf.base);
    req->buf.len = len;

    req->session = self; /* remember the client pointer to used in __on_uv_send_done */

    memcpy(req->buf.base, buf, len);

    ret = uv_udp_send((uv_udp_send_t*)req, self->udp, &req->buf, 1, &self->addr, __on_uv_send_done);
    if (ret < 0)
        goto error;
    return ret;

    error:
    if (req && req->buf.base)
        free(req->buf.base);
    if (req)
        free(req);
    return -1;
}


inline etp_client_shared_t *etp_client_shared_alloc(void)
{
    etp_client_shared_t *client = (etp_client_shared_t *)malloc(sizeof(etp_client_shared_t));
    SG_ASSERT_MALLOC(client);
    memset(client, 0, sizeof(etp_client_shared_t));
    return client;
}

inline int etp_client_shared_set_callbacks(etp_client_shared_t *self,
                                  sg_etp_on_open_func_t on_open, sg_etp_on_recv_func_t on_recv,
                                  sg_etp_on_sent_func_t on_sent, sg_etp_on_close_func_t on_close)
{
    self->on_open = on_open;
    self->on_recv = on_recv;
    self->on_sent = on_sent;
    self->on_close = on_close;

    return SG_OK;
}


inline bool etp_client_shared_start(etp_client_shared_t *self, IUINT32 conv,
        const struct sockaddr *addr, uv_loop_t *loop, uv_udp_t *udp, int interval_ms, void *user_data)
{
    int ret = ERROR;

    SG_ASSERT(self->on_open);
    SG_ASSERT(self->on_recv);
    SG_ASSERT(self->on_sent);
    SG_ASSERT(self->on_close);

    /* init normal parameters */
    self->conv = conv;
    memcpy(&(self->addr), addr, sizeof(struct sockaddr));
    self->user_data = user_data;
    self->uv_timer_interval_ms = interval_ms;
    self->speed_counter = sg_speed_counter_open(1000);
    SG_ASSERT(self->speed_counter);

    /* init loop */
    if (loop) {
        self->alloc_loop = false;
        self->loop = loop;
    } else {
        self->alloc_loop = true;
        self->loop = (uv_loop_t *)malloc(sizeof(struct uv_loop_s));
        uv_loop_init(self->loop);
    }

    /* init libuv udp handle */
    if (udp) {
        self->alloc_udp = false;
        self->udp = udp;
    } else {
        self->alloc_udp = true;
        self->udp = (uv_udp_t *)malloc(sizeof(uv_udp_s));
        ret = uv_udp_init(self->loop, self->udp);
        SG_ASSERT_MSG(ret >= 0, "init udp failed");
        self->udp->data = self;
        ret = uv_udp_recv_start(self->udp, uv_comm_on_alloc_buf, __on_uv_recv_udp);
        SG_ASSERT_MSG(ret >= 0, "start udp recv failed");
    }

    /* create and init kcp object */
    self->kcp = ikcp_create(self->conv, (void *)self);
    SG_ASSERT(self->kcp);
    self->kcp->output = __on_kcp_output;
    self->conn_timeout_ms  = SG_ETP_SESSION_TIMEOUT;
    self->recv_timeout_timing = uv_now(self->loop) + self->conn_timeout_ms;
    ret = ikcp_nodelay(self->kcp, 1, interval_ms, 2, 0); /* ultra fast configuration */
    SG_ASSERT_MSG(ret >= 0, "ikcp nodelay failed");

    /* start a timer for kcp update and receiving */
    ret = uv_timer_init(self->loop, &(self->timer));
    SG_ASSERT_MSG(ret >= 0, "init timer failed");
    self->timer.data = self; /* link client pointer to timer */
    ret = uv_timer_start(&(self->timer), __on_uv_timer, self->uv_timer_interval_ms, self->uv_timer_interval_ms);
    SG_ASSERT_MSG(ret >= 0, "start timer failed");

    /* start idle monitor */
    uv_idle_init(self->loop, &(self->idler));
    self->idler.data = self;

    self->on_open(self);
    return true;
}

inline sg_err_t etp_client_shared_send(etp_client_shared_t *self, const void *data, size_t size)
{
    self->kcp_next_update_time = 0; /* clear to update kcp in next update period */
    return (ikcp_send(self->kcp, data, size) < 0) ? SG_ERR : SG_OK;
}

inline int etp_client_shared_recv_udp(etp_client_shared_t *self, void *data, size_t size)
{
    uint64_t now = uv_now(self->loop);

    sg_speed_counter_reg(self->speed_counter, size);

    self->recv_timeout_timing = now + self->conn_timeout_ms;

    self->kcp_next_update_time = 0; /* clear to update kcp in next update period */
    return ikcp_input(self->kcp, data, (long)size);
}

inline void etp_client_shared_set_max_send_speed(etp_client_shared_t *self, size_t kbps)
{
    self->max_limit_speed = kbps;
}

inline bool etp_client_shared_close(etp_client_shared_t *self)
{
    sg_log_dbg("ikcp_peeksize: %d, ikcp_waitsnd: %d",
               ikcp_peeksize(self->kcp), ikcp_waitsnd(self->kcp));

    /* if there is data waiting to be sent or received in queue */
    if (ikcp_waitsnd(self->kcp) > 0 || ikcp_peeksize(self->kcp) > 0) {
        self->delay_close = true; /* mark for close later, not right now */
        return true;
    }

    /* cleanup */
    sg_log_dbg("close client\n");
    ikcp_release(self->kcp);
    uv_timer_stop(&(self->timer));
    if (self->alloc_udp) {
        uv_close((uv_handle_t *)self->udp, uv_common_on_close_done);
        free(self->udp);
    }
    if (self->alloc_loop) {
        uv_loop_close(self->loop);
        free(self->loop);
    }
    if (self->recv_buf)
        free(self->recv_buf);
    if (self->recv_speed_counter)
        sg_speed_counter_close(self->recv_speed_counter);
    if (self->send_speed_counter)
        sg_speed_counter_close(self->send_speed_counter);
    self->on_close(self, SG_OK, "ok");
    free(self);

    return true;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_ETP_CLIENT_SHARED_H */
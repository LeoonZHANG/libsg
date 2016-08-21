/**
 * etp.h
 * ETP (extreme transfer protocol) is a transfer protocol based on LIBUV / UDP / KCP and FEC.
 */
 
#include "uv.h"
#include "ikcp.h"
#include "etp.h"

#if defined(linux) || defined(__linux) || defined(__linux__)
#   ifndef PLATFORM_LINUX
#       define PLATFORM_LINUX
#   endif
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#   ifndef PLATFORM_WINDOWS
#       define PLATFORM_WINDOWS
#   endif
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#   ifndef PLATFORM_MACOS
#       define PLATFORM_MACOS
#   endif
#else
#   error Unsupported platform.
#endif

#if defined(PLATFORM_WINDOWS)
#   include <winsock2.h>
#   include <windows.h>
#   pragma comment(lib ,"ws2_32.lib")
#   pragma comment(lib, "psapi.lib")
#   pragma comment(lib, "Iphlpapi.lib")
#   pragma comment(lib, "userenv.lib")
#elif defined(PLATFORM_LINUX)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/epoll.h>
#   include <sys/time.h>
#elif defined(PLATFORM_MACOS) || defined(PLATFORM_BSD)
#   include <sys/event.h>
#   include <sys/socket.h>
#   include <arpa/inet.h>
#   include <sys/types.h>
#   include <sys/event.h>
#   include <sys/time.h>
#endif

#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned char bool;
#define true    1
#define false   0

#define OK      0
#define ERROR (-1)

#define SG_ASSERT(exp, prmpt)          if (exp) {} else { printf("%d:%s() " prmpt "\n", __LINE__, __FUNCTION__); return; }
#define SG_ASSERT_RET(exp, prmpt, ret) if (exp) {} else { printf("%d:%s() " prmpt "\n", __LINE__, __FUNCTION__); return(ret); }
#define SG_ASSERT_BRK(exp, prmpt)      if (exp) {} else { printf("%d:%s() " prmpt "\n", __LINE__, __FUNCTION__); break; }

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

typedef struct sg_etp_real
{
    uint32_t          conv;
    uv_loop_t         *loop;
    uv_udp_t          udp;
    uv_timer_t        timer;
    uv_idle_t         idle;
    struct sockaddr   addr;
    ikcpcb            *kcp;
    IUINT32           next_update;
    bool              to_close;
    sg_etp_on_open_func_t  on_open;
    sg_etp_on_data_func_t  on_data;
	sg_etp_on_sent_func_t  on_sent;
    sg_etp_on_close_func_t on_close;
	void                   *data;

    /* 统计速率相关,采用滑窗试计算 */
    uint64_t            last_time; /* 上一次统计时间 */
    int                 last_send_byte;
    int                 max_speed_limit;
    int                 head, tail;
    int                 last_speed[SG_ETP_SPEED_STAT_SAMPLE_COUNT];
    double              current_speed;
}sg_kcp_t;

typedef struct send_req_s
{
	uv_udp_send_t req;
	uv_buf_t buf;
    sg_kcp_t* client; /* send_done怎么知道这个数据是谁发，给谁发的？加上client，用于统计流量，限制发送速度时，要通过这个client关联。 */
}send_req_t;

static void on_uv_alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf);
static void on_client_recv_udp(
    uv_udp_t                *handle,
    ssize_t                 nread,
    const uv_buf_t          *rcvbuf,
    const struct sockaddr   *addr,
    unsigned int            flags);
/* for libuv */
static void on_uv_close_done(uv_handle_t *handle);
/* for kcp callback */
static int on_kcp_output(const char *buf, int len, struct IKCPCB *kcp, void *user);
/* for libuv callback */
static void on_udp_send_done(uv_udp_send_t *req, int status);

/* for update client send speed result */
static void sg_kcp_update_speed(sg_kcp_t* client, uint64_t now);

/* for libuv */
static void on_uv_alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
	buf->len = (unsigned long)size;
	buf->base = malloc(size);
}

/* for libuv */
static void on_client_recv_udp(uv_udp_t *handle, ssize_t nread,
        const uv_buf_t *rcvbuf, const struct sockaddr *addr, unsigned int flags)
{
	sg_etp_t *client = NULL;

	/*printf("recv udp %d\n", nread);*/

    do {
        /*SG_ASSERT_BRK(nread > 0, "no data recv");*/
        if (nread <= 0)
            break;

    	client = (sg_etp_t *)(handle->data);
        /*memcpy(&(client->addr), addr, sizeof(struct sockaddr));*/
        client->next_update = 0; /* clear next update */
    	ikcp_input(client->kcp, rcvbuf->base, (long)nread);
    } while (0);

	free(rcvbuf->base);
}

/* for libuv */
static void on_uv_close_done(uv_handle_t *handle)
{
}

/* for libuv */
static void on_uv_timer_cb(uv_timer_t *handle)
{
    sg_etp_t * client = handle->data;
    IUINT32 now = 0;

    /*printf("update %d\n", client->conv);*/
    
    /* update ikcp */
    now = (IUINT32)uv_now(client->loop);
    sg_kcp_update_speed(client, now);
    if (now >= client->next_update)
    {
        ikcp_update(client->kcp, now);
        client->next_update = ikcp_check(client->kcp, now);
    }

    if (client->to_close)
        sg_etp_close(client);
}

static void on_uv_idle_cb(uv_idle_t *handle)
{
    int ret = 0;
    int len = 0;
    char *data = NULL;
    sg_etp_t *client = NULL;

    client = handle->data;

    /* recv data from kcp level */
    len = ikcp_peeksize(client->kcp);
	if (len < 0)
		return;

	data = (char *)malloc(len);
	ret = ikcp_recv(client->kcp, data, len);
	if (ret < 0) {
        printf("recv ikcp failed, ret = %d\n", ret);
		free(data);
		return;
	}

    client->on_data(client, data, len);

    free(data);
    data = NULL;
}

/* for kcp callback */
static int on_kcp_output(const char *buf, int len, struct IKCPCB *kcp, void *user)
{
	sg_kcp_t *client = (sg_kcp_t *)user;
    int ret = -1;
    send_req_t *req = NULL;

    /*printf("udp send: %d\n", len);*/

    do {
        /* 限速 */
        if (client->max_speed_limit > 0 && client->current_speed > client->max_speed_limit)
            return ret;
			
    	req = (send_req_t *)malloc(sizeof(send_req_t));
        SG_ASSERT_BRK(NULL != req, "create send_req_t failed");

        memset(req, 0, sizeof(send_req_t));

    	req->buf.base = malloc(sizeof(char) *len);
        SG_ASSERT_BRK(NULL != req->buf.base, "create buf failed");
    	req->buf.len = len;
        req->client = client;

    	memcpy(req->buf.base, buf, len);

    	ret = uv_udp_send((uv_udp_send_t*)req, &(client->udp), &req->buf, 1, &client->addr, on_udp_send_done);
    	if (ret < 0) {
    		free(req->buf.base); /* TODO: ensure free */
    		free(req); /* TODO: ensure free ? */
    		return -1;
    	}

    	return ret;
    } while (0);

    if (NULL != req) {
        if (NULL != req->buf.base) {
            free(req->buf.base);
            req->buf.base = NULL;
        }
        
        free(req);
        req = NULL;
    }

	return ret;
}

/* for libuv callback */
static void on_udp_send_done(uv_udp_send_t *req, int status)
{
	send_req_t * send_req = (send_req_t *)req;
    send_req->client->on_sent(send_req->client, status);
    send_req->client->last_send_byte += send_req->buf.len; /* 统计网速需要 */
	free(send_req->buf.base); /* TODO: ensure free*/
	free(send_req);	/** TODO: ensure free */
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
    sg_kcp_t * client = NULL;
    struct timeval tv;
    struct sockaddr_in addr;
	int ret = -1;

    do {
        /* create the client object */
        client = malloc(sizeof(sg_etp_t));
        SG_ASSERT_BRK(NULL != client, "create client failed");

        client->on_open  = on_open;
        client->on_data  = on_data;
		client->on_sent = on_sent;
        client->on_close = on_close;

		/* 原来client->conv=client，现在改成时间，因为反复关开此程序进程，client指针可能相同。 */
        gettimeofday(&tv, NULL);
        client->conv = (uint32_t)(tv.tv_sec * 1000000 + tv.tv_usec); /* TODO: alloc the conversation id */

    	/* get address */
    	ret = uv_ip4_addr(server_addr, server_port, &addr);
    	SG_ASSERT_BRK(ret >= 0, "get address failed");
        memcpy(&(client->addr), &addr, sizeof(struct sockaddr));

        /* create the kcp object */
    	client->kcp = ikcp_create(client->conv, (void*)client);
    	SG_ASSERT_BRK(NULL != client->kcp, "create ikcp failed");

    	client->kcp->output = on_kcp_output;

    	client->loop = uv_default_loop();

        client->on_open(client);

        return client;
    } while (0);

    if (NULL != client) {
        free(client);
        client = NULL;
    }
    return client;
}

int sg_etp_run(sg_etp_t *client, int interval_ms)
{
    int ret = -1;
    
    ret = ikcp_nodelay(client->kcp, 1, interval_ms, 2, 1);
    SG_ASSERT_RET(ret >= 0, "ikcp nodelay failed", ERROR);

    /* init udp */
	ret = uv_udp_init(client->loop, &(client->udp));
    SG_ASSERT_RET(ret >= 0, "init udp failed", ERROR);
    client->udp.data = client;
	ret = uv_udp_recv_start(&(client->udp), on_uv_alloc_buffer, on_client_recv_udp);
	SG_ASSERT_RET(ret >= 0, "start udp recv failed", ERROR);

    /* start a timer for kcp update and receiving */
    ret = uv_timer_init(client->loop, &(client->timer));
    SG_ASSERT_RET(ret >= 0, "init timer failed", ERROR);
    client->timer.data = client; /* link client pointer to timer */
    ret = uv_timer_start(&(client->timer), on_uv_timer_cb, interval_ms, interval_ms);
    SG_ASSERT_RET(ret >= 0, "start timer failed", ERROR);

    /* reg idle for data process */
    ret = uv_idle_init(client->loop, &(client->idle));
    SG_ASSERT_RET(ret >= 0, "init idle failed", ERROR);
    client->idle.data = client;
    ret = uv_idle_start(&(client->idle), on_uv_idle_cb);
    SG_ASSERT_RET(ret >= 0, "start idle failed", ERROR);

    /* enter loop */
    uv_run(client->loop, UV_RUN_DEFAULT);

    return OK;
}

int sg_etp_send(sg_etp_t *client, const void *data, uint64_t size)
{
    client->next_update = 0; /* clear next update */
    return ikcp_send(client->kcp, data, size);
}

uint32_t sg_etp_now(sg_etp_t *client)
{
    return (uint32_t)uv_now(client->loop);
}

void sg_etp_close(sg_etp_t *client)
{
    if (ikcp_waitsnd(client->kcp) > 0 || ikcp_peeksize(client->kcp) > 0) {
        client->to_close = true; /* mark for close later */
        return;
    }

    client->on_close(client, 0, "");

    ikcp_release(client->kcp);

    uv_timer_stop(&(client->timer));
    uv_idle_stop(&(client->idle));

    uv_close((uv_handle_t *)&(client->udp), on_uv_close_done);
   
    free(client);
}

void sg_etp_free(void)
{

}

/* Update send speed result. */
static void sg_kcp_update_speed(sg_kcp_t* client, uint64_t now)
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
            client->tail &= (SG_ETP_SPEED_STAT_SAMPLE_COUNT - 1);
            if (client->tail == client->head)
                client->head = (client->head + 1)&(SG_ETP_SPEED_STAT_SAMPLE_COUNT - 1);

            speed = 0;
            for (int i = client->head; i != client->tail; i=(i+1)&(SG_ETP_SPEED_STAT_SAMPLE_COUNT - 1)) {
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

void sg_etp_set_max_send_speed(sg_etp_t *client, size_t kbps)
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
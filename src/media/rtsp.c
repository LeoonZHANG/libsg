/**
 * rtsp.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <unistd.h> /* usleep */
#include "../../include/sg/media/rtsp.h"

typedef int bool;
#define true 1
#define false 0

static const char *rtsp_transport_tcp = "RTP/AVP/TCP;interleaved=0-1";
static const char *rtsp_transport_udp = "RTP/AVP;unicast;client_port=%d-%d";

struct sg_rtsp_real {
    char *url;
    char *ctrl_attr; /* control attribute */
    char *uri;       /* uri = url + "/" + ctrl_attr */
    CURL *curl;
    unsigned int udp_client_port;
    enum sg_rtsp_data_protocol prot; /* tcp / udp */
    sg_rtsp_on_recv_func_t on_recv;
    sg_rtsp_on_close_func_t on_close;
    void *context; /* user data for callbacks */
};


/* return：1:表示一帧结束  0:FU-A分片未结束或帧未结束 */
static bool rtp_unpack_h264(void *rtp_buf, int rtp_buf_len, void **h264_buf, int *h264_buf_len)
{
    #define RTP_HEADLEN 12
    unsigned char *src    = (unsigned char *)rtp_buf + RTP_HEADLEN;
    unsigned char head1   = *src;         // 获取RTP head之后的第一个字节
    unsigned char head2   = *(src + 1 );  // 获取RTP head之后的第二个字节
    unsigned char nal     = head1 & 0x1f; // 获取FU indicator的类型域，
    unsigned char flag    = head2 & 0xe0; // 获取FU header的前三位，判断当前是分包的开始、中间或结束
    unsigned char nal_fua = (head1 & 0xe0) | (head2 & 0x1f); // FU_A nal
    bool  b_finish_frame  = false;
    unsigned char *tmp;

    *h264_buf_len = 0;
    if (rtp_buf_len < RTP_HEADLEN)
        return false;
    
    if (nal == 0x1c) { // 判断NAL的类型为0x1c=28，说明是FU-A分片
        if (flag == 0x80) { // 开始
            *h264_buf = src - 3;
            *((int *)(*h264_buf)) = 0x01000000; // zyf:大模式会有问题
            *((char *)(*h264_buf) + 4) = nal_fua;
            *h264_buf_len = rtp_buf_len - RTP_HEADLEN + 3;
        } else if (flag == 0x40) { // 结束
            *h264_buf = src + 2;
            *h264_buf_len = rtp_buf_len - RTP_HEADLEN - 2;
        } else { // 中间
            *h264_buf = src + 2;
            *h264_buf_len = rtp_buf_len - RTP_HEADLEN - 2;
        }
    } else { // 单包数据
        *h264_buf = src - 4;
        *((int *)(*h264_buf)) = 0x01000000; // zyf:大模式会有问题
        *h264_buf_len = rtp_buf_len - RTP_HEADLEN + 4;
    }

    tmp = (unsigned char *)rtp_buf;
    if (tmp[1] & 0x80 )
        b_finish_frame = true; // rtp mark
    else
        b_finish_frame = false;
    return b_finish_frame;
}

/* receive RTP packet */
static size_t interleave_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;
    void *h264_hd = NULL;
    int h264_len = 0;

    printf("recv interleave data %ld. first char 0x%02x.\n", size * nmemb, ((unsigned char *)ptr)[0]);

    /* return RTP packet to user */
    r->on_recv((sg_rtsp_t *)r, ptr, size * nmemb, r->context);
return size * nmemb;
    rtp_unpack_h264(ptr, size * nmemb, &h264_hd, &h264_len);
    r->on_recv((sg_rtsp_t *)r, h264_hd, size * nmemb - (h264_hd - ptr), r->context);

    return size * nmemb;
}

/* receive rtp text info after rtsp play request */
static size_t play_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;
    printf("play_callback: %s\n", (const char *)ptr);
    return size * nmemb;
}

/* find out its control attribute */
static size_t describe_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;
    char tmp[1024];
    char *ctrl_attr_hd = NULL;
    char *ctrl_attr_tail = NULL;
    const char *ctrl_msg = "a=control:";
    const char *ctrl_msg_err = "a=control:*";

    ctrl_attr_hd = (const char *)ptr;
    while (1) {
        ctrl_attr_hd = strstr(ctrl_attr_hd, ctrl_msg);
        if (!ctrl_attr_hd)
            break;
        if (strstr(ctrl_attr_hd, ctrl_msg_err)) {
            ctrl_attr_hd++;
            continue;
        } else {
            ctrl_attr_tail = strstr(ctrl_attr_hd, "\r");
            if (!ctrl_attr_tail)
                ctrl_attr_tail = strstr(ctrl_attr_hd, "\n");
            if (!ctrl_attr_tail)
                ctrl_attr_tail = ((char *)ptr) + size * nmemb;
            ctrl_attr_hd += strlen(ctrl_msg);
            break;
        }
    }

    if (ctrl_attr_hd && ctrl_attr_tail) {
        strncpy(tmp, ctrl_attr_hd, ctrl_attr_tail - ctrl_attr_hd);
        tmp[ctrl_attr_tail - ctrl_attr_hd] = 0;
        //r->ctrl_attr = strdup(tmp);
        //printf("control attr found:%s.\n", r->ctrl_attr);
    }

    return size * nmemb;
}

int sg_rtsp_init(void)
{
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    return (res == CURLE_OK) ? 0 : -1;
}

sg_rtsp_t *sg_rtsp_open(const char *url, unsigned int udp_client_port, enum sg_rtsp_data_protocol prot,
                        sg_rtsp_on_recv_func_t on_recv, sg_rtsp_on_close_func_t on_close,
                        void *context)
{
    struct sg_rtsp_real *r;
    CURLcode res = CURLE_OK;
    size_t new_uri_len;
    char transport[1024] = {0};

    r = (struct sg_rtsp_real *)malloc(sizeof(struct sg_rtsp_real));
    if (!r) {
        printf("malloc errpr\n");
        return NULL;
    }
    memset(r, 0, sizeof(struct sg_rtsp_real));

    r->udp_client_port  = udp_client_port;
    r->prot             = prot;
    r->on_recv          = on_recv;
    r->on_close         = on_close;
    r->context          = context;
    r->curl             = curl_easy_init();
    r->url              = strdup(url);
    r->uri              = strdup(url);
    if(!r->curl || !r->url || !r->uri || prot < SGRTSPDATAPROTOCOL_TCP || prot > SGRTSPDATAPROTOCOL_UDP) {
        printf("sg_rtsp_real member invalid\n");
        goto err_exit;
    }

    /* init curl session */
    curl_easy_setopt(r->curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(r->curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(r->curl, CURLOPT_HEADERDATA, stdout);
    curl_easy_setopt(r->curl, CURLOPT_URL, r->url);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK) {
        printf("RTSP session init error, error num %d, url %s\n", res, r->url);
        goto err_exit;
    }

    /* RTSP REQ: OPTIONS */
    curl_easy_setopt(r->curl, CURLOPT_RTSP_STREAM_URI, r->uri);
    curl_easy_setopt(r->curl, CURLOPT_INTERLEAVEFUNCTION, interleave_callback);
    curl_easy_setopt(r->curl, CURLOPT_INTERLEAVEDATA, (void *)r);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_OPTIONS);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK) {
        printf("RTSP OPTIONS error\n");
        goto err_exit;
    }

    /* RTSP REQ: DESCRIBE */
    curl_easy_setopt(r->curl, CURLOPT_WRITEFUNCTION, describe_callback);
    curl_easy_setopt(r->curl, CURLOPT_WRITEDATA, (void *)r);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_STREAM_URI, r->uri);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_DESCRIBE);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK) {
        printf("RTSP DESCRIBE error\n");
        goto err_exit;
    }
    curl_easy_setopt(r->curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(r->curl, CURLOPT_WRITEDATA, NULL);

    /* RTSP REQ: SETUP */
    if (r->prot == SGRTSPDATAPROTOCOL_TCP)
        snprintf(transport, 1024, "%s", rtsp_transport_tcp);
    else
        snprintf(transport, 1024, rtsp_transport_udp, r->udp_client_port, r->udp_client_port + 1);
    if (r->ctrl_attr && strlen(r->ctrl_attr) > 0) { /* generate new uri */
        new_uri_len = strlen(r->url) + strlen(r->ctrl_attr) + 2;
        free(r->uri);
        r->uri = NULL;
        r->uri = (char *)malloc(new_uri_len);
        if (!r->uri) {
            printf("malloc error\n");
            goto err_exit;
        }
        snprintf(r->uri, new_uri_len, "%s/%s", r->url, r->ctrl_attr);
        printf("new uri:%s.\n", r->uri);
    }
    curl_easy_setopt(r->curl, CURLOPT_RTSP_TRANSPORT, transport);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_STREAM_URI, r->uri);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_SETUP);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK) {
        printf("RTSP SETUP error\n");
        goto err_exit;
    }

success_exit:
    return (sg_rtsp_t *)r;

err_exit:
    sg_rtsp_close(r);
    return NULL;
}

int sg_rtsp_play(sg_rtsp_t *r)
{
    CURLcode res;
    struct sg_rtsp_real *rp = (struct sg_rtsp_real *)r;

    /* RTSP REQ: PLAY */
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_STREAM_URI, rp->url); /* attention: use uri here will cause error */
    curl_easy_setopt(rp->curl, CURLOPT_RANGE, "0.000-");
    curl_easy_setopt(rp->curl, CURLOPT_WRITEFUNCTION, play_callback);
    curl_easy_setopt(rp->curl, CURLOPT_WRITEDATA, (void *)rp);
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_PLAY);
    res = curl_easy_perform(rp->curl);
    if (res == CURLE_OK)
        printf("RTSP play success\n");
    else {
        printf("RTSP play error\n");
        return -1;
    }

    /* RTSP REQ: RECEIVE */
    while (1) {
        curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_RECEIVE);
        res = curl_easy_perform(rp->curl);
        if (res != CURLE_OK) {
            printf("RTSP receive error\n");
            break;
        }
        usleep(1000); /* sleep 1 ms */
    }

    return 0;
}

int sg_rtsp_pause(sg_rtsp_t *r)
{
    CURLcode res;
    struct sg_rtsp_real *rp = (struct sg_rtsp_real *)r;

    /* RTSP REQ: PAUSE */
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_STREAM_URI, rp->uri);
    curl_easy_setopt(rp->curl, CURLOPT_RANGE, "0.000-");
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_PAUSE);
    res = curl_easy_perform(rp->curl);
    if (res == CURLE_OK)
        printf("RTSP pause now!\n");
    else
        printf("RTSP pause error!\n");
    return (res == CURLE_OK) ? 0 : -1;
}

void sg_rtsp_close(sg_rtsp_t *r)
{
    struct sg_rtsp_real *rp = (struct sg_rtsp_real *)r;

    if (!rp)
        return;

    /* RTSP REQ: TEARDOWN */
    if (rp->curl) {
        curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_TEARDOWN);
        curl_easy_perform(rp->curl);
        curl_easy_cleanup(rp->curl);
    }

    if (rp->url)
        free(rp->url);
    if (rp->ctrl_attr)
        free(rp->ctrl_attr);
    if (rp->uri)
        free(rp->uri);
    free(rp);
}

void sg_rtsp_cleanup(void)
{
    curl_global_cleanup();
}

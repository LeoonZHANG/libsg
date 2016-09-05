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

/* receive RTP packet */
static size_t interleave_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;

    printf("recv interleave data %ld. first char 0x%02x.\n", size * nmemb, ((unsigned char *)ptr)[0]);

    /* return RTP packet to user */
    r->on_recv((sg_rtsp_t *)r, ptr, size * nmemb, r->context);

    return size * nmemb;
}

/* receive rtp text info after rtsp play request */
static size_t play_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;
    /* printf("play_callback: %s\n", (const char *)ptr); */
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
        r->ctrl_attr = strdup(tmp);
        printf("control attr found:%s.\n", r->ctrl_attr);
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
    if (!r)
        return NULL;
    memset(r, 0, sizeof(struct sg_rtsp_real));

    r->udp_client_port  = udp_client_port;
    r->prot             = prot;
    r->on_recv          = on_recv;
    r->on_close         = on_close;
    r->context          = context;
    r->curl             = curl_easy_init();
    r->url              = strdup(url);
    r->uri              = strdup(url);
    if(!r->curl || !r->url || !r->uri || prot < SGRTSPDATAPROTOCOL_TCP || prot > SGRTSPDATAPROTOCOL_UDP)
        goto err_exit;

    /* init curl session */
    curl_easy_setopt(r->curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(r->curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(r->curl, CURLOPT_HEADERDATA, stdout);
    curl_easy_setopt(r->curl, CURLOPT_URL, r->url);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK)
        goto err_exit;

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
        if (!r->uri)
            goto err_exit;
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
        printf(stdout, "RTSP pause now!\n");
    else
        printf(stderr, "RTSP pause error!\n");
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
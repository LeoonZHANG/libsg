/**
 * rtsp.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <sg/media/rtsp.h>

static const char *rtsp_transport_tcp = "RTP/AVP/TCP;unicast;interleaved=0-1";
static const char *rtsp_transport_udp = "RTP/AVP;unicast;client_port=%d-%d";

struct sg_rtsp_real {
    char *url;
    char *ctrl_attr; /* control attribute */
    char *uri;       /* uri = url + "/" + ctrl_attr */
    CURL *curl;
    unsigned int udp_client_port;
    bool use_tcp;
    sg_rtsp_on_open_func_t on_open;
    sg_rtsp_on_recv_func_t on_recv;
    sg_rtsp_on_close_func_t on_close;
    void *context; /* user data for callbacks */
};

/* receive RTP packet */
size_t interleave_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;

    /* return RTP packet to user */
    r->on_recv((sg_rtsp_t *)r, ptr, size * nmemb, r->context);

    /* continue to receive RTP packet */
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_RECEIVE);
    my_curl_easy_perform(r->curl);

    return size * nmemb;
}

/* receive rtp text info after rtsp play request */
size_t play_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;
    return size * nmemb;
}

/* find out its control attribute */
size_t describe_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    char *line = strtok((char *)ptr, "\r\n");
    struct sg_rtsp_real *r = (struct sg_rtsp_real *)userdata;
    char tmp[1024];

    while (line) {
        if (sscanf(line, "a=control:%s", tmp) == 1) {
            r->ctrl_attr = strdup(tmp);
            break;
        }
        line = strtok(NULL, "\r\n");
    }

    return size * nmemb;
}



int sg_rtsp_init(void)
{
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res == CURLE_OK)
        return 0;
    else
        return -1;
}

sg_rtsp_t *sg_rtsp_open(const char *url, unsigned int udp_client_port, bool use_tcp,
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

    r->udp_client_port = udp_client_port;
    r->use_tcp         = use_tcp;
    r->on_recv         = on_recv;
    r->on_close        = on_close;
    r->context         = context;
    r->curl            = curl_easy_init();
    r->url             = strdup(url);
    r->uri      = strdup(url);
    if(!r->curl || !r->url || !r->uri)
        goto err_exit;

    /* init curl session */
    curl_easy_setopt(r->curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(r->curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(r->curl, CURLOPT_HEADERDATA, stdout);
    curl_easy_setopt(r->curl, CURLOPT_URL, r->url);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK)
        goto err_exit;

    /* RTSP CMD: OPTIONS */
    curl_easy_setopt(r->curl, CURLOPT_RTSP_STREAM_URI, r->uri);
    curl_easy_setopt(r->curl, CURLOPT_INTERLEAVEFUNCTION, interleave_callback);
    curl_easy_setopt(r->curl, CURLOPT_INTERLEAVEDATA, (void *)r);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_OPTIONS);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK)
        goto err_exit;

    /* RTSP CMD: DESCRIBE */
    curl_easy_setopt(r->curl, CURLOPT_WRITEFUNCTION, describe_callback);
    curl_easy_setopt(r->curl, CURLOPT_WRITEDATA, (void *)r);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_STREAM_URI, r->uri);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_DESCRIBE);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK)
        goto err_exit;
    curl_easy_setopt(r->curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(r->curl, CURLOPT_WRITEDATA, NULL);

    /* RTSP CMD: SETUP */
    if (r->use_tcp)
        snprintf(transport, 1024, "%s", rtsp_transport_tcp);
    else
        snprintf(transport, 1024, rtsp_transport_udp, r->udp_client_port, r->udp_client_port + 1);
    if (r->ctrl_attr && strlen(r->ctrl_attr) > 0) { /* generate new uri */
        new_uri_len = strlen(r->url) + strlen(r->ctrl_attr) + 1;
        free(r->uri);
        r->uri = NULL;
        r->uri = (char *)malloc(new_uri_len);
        if (!r->uri)
            goto err_exit;
        snprintf(r->uri, new_uri_len, "%s/%s", r->url, r->ctrl_attr);
    }
    curl_easy_setopt(r->curl, CURLOPT_RTSP_TRANSPORT, transport);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_STREAM_URI, r->uri);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_SETUP);
    res = curl_easy_perform(r->curl);
    if (res != CURLE_OK)
        goto err_exit;

success:
    return (sg_rtsp_t *)r;

err_exit:
    sg_rtsp_close(r);
    return NULL;
}

int sg_rtsp_play(sg_rtsp_t *r)
{
    CURLcode res;
    struct sg_rtsp_real *rp = (struct sg_rtsp_real *)r;

    /* RTSP CMD: PLAY */
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_STREAM_URI, rp->uri);
    curl_easy_setopt(rp->curl, CURLOPT_RANGE, "0.000-");
    curl_easy_setopt(rp->curl, CURLOPT_WRITEFUNCTION, play_callback);
    curl_easy_setopt(rp->curl, CURLOPT_WRITEDATA, (void *)rp);
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_PLAY);
    res = curl_easy_perform(rp->curl);
    if (res == CURLE_OK)
        printf(stdout, "RTSP play now!\n");
    else {
        printf(stderr, "RTSP play error!\n");
        return -1;
    }

    /* RTSP CMD: RECEIVE */
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_RECEIVE);
    res = curl_easy_perform(rp->curl);

    return (res == CURLE_OK) ? 0 : -1;
}

int sg_rtsp_pause(sg_rtsp_t *r)
{
    CURLcode res;
    struct sg_rtsp_real *rp = (struct sg_rtsp_real *)r;

    /* RTSP CMD: PAUSE */
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_STREAM_URI, rp->uri);
    curl_easy_setopt(rp->curl, CURLOPT_RANGE, "0.000-");
    curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_PAUSE);
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

    /* RTSP CMD: TEARDOWN */
    if (rp->curl) {
        curl_easy_setopt(rp->curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_TEARDOWN);
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
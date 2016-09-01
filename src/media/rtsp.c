/**
 * rtsp.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <curl/curl.h>
#include <sg/media/rtsp.h>

static const char *rtsp_transport_tcp = "RTP/AVP/TCP;unicast;client_port=1234-1235";
static const char *rtsp_transport_udp = "RTP/AVP;unicast;client_port=1234-1235";

struct sg_rtsp_real {
    char *url;
    CURL *curl;
    char *sdp_filename;
    bool use_tcp;
    char *control;
    char *range;
    sg_rtsp_on_open_func_t on_open;
    sg_rtsp_on_data_func_t on_data;
    sg_rtsp_on_close_func_t on_close;
};

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct sg_rtsp_real *r = (struct sg_rtsp_real *) userdata;
    r->on_data((sg_rtsp_t *) r, ptr, size * nmemb);
    return size * nmemb;
}

static void get_media_control_attribute(sg_rtsp_t *rtsp)
{

    int max_len = 256;
    char *s = malloc(max_len);
    FILE *sdp_fp = fopen(rtsp->sdp_filename, "rb");
    if (sdp_fp == NULL)
        printf("no file\n");
    rtsp->control[0] = '\0';
    if (sdp_fp != NULL) {
        while (fgets(s, max_len - 2, sdp_fp) != NULL) {
            printf("sscanf call\n");
            sscanf(s, " a = control: %s", rtsp->control);
            printf("sss %s\n", s);
        }
        printf("control %s\n", rtsp->control);
        fclose(sdp_fp);
    }
    free(s);
}

static void get_sdp_filename(const char *url, char *sdp_filename, size_t namelen) {
    const char *s = strrchr(url, '/');
    strcpy(sdp_filename, "video.sdp");
    if (s != NULL) {
        s++;
        if (s[0] != '\0')
            snprintf(sdp_filename, namelen, "%s.sdp", s);
    }
}







int sg_rtsp_init(void)
{
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res == CURLE_OK)
        return 0;
    else
        return -1;
}

sg_rtsp_t *sg_rtsp_open_url(const char *url, bool use_tcp,
                            sg_rtsp_on_open_func_t on_open,
                            sg_rtsp_on_data_func_t on_data,
                            sg_rtsp_on_close_func_t on_close)
{

    sg_rtsp_t *r;

    r = (sg_rtsp_t *)malloc(sizeof(struct sg_rtsp_real));
    if (!r)
        return NULL;
    memset(r, 0, sizeof(struct sg_rtsp_real));

    r->use_tcp  = use_tcp;
    r->on_open  = on_open;
    r->on_data  = on_data;
    r->on_close = on_close;
    r->curl     = curl_easy_init();
    r->url      = strdup(url);
    if(!r->curl || !r->url) {
        if (r->curl)
            curl_easy_cleanup(r->curl);
        if (r->url)
            free(r->url);
        free(r);
        return NULL;
    }

    curl_easy_setopt(r->curl, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(r->curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(r->curl, CURLOPT_HEADERDATA, stdout);
    curl_easy_setopt(r->curl, CURLOPT_URL, r->url);
    curl_easy_perform(r->curl);
    r->uri = (char *)malloc(strlen(url) + 32);
    snprintf(r->uri, strlen(rtsp->url) + 32, "%s", r->url);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_STREAM_URI, r->uri);
    curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, (long) CURL_RTSPREQ_OPTIONS);
    curl_easy_perform(r->curl);

    return r;
}

sg_rtsp_t *sg_rtsp_open_sdp(const char *url,
                            sg_rtsp_on_open_func_t on_open,
                            sg_rtsp_on_data_func_t on_data,
                            sg_rtsp_on_close_func_t on_close)
{

    sg_rtsp_t *rtsp;
    rtsp = (sg_rtsp_t *) malloc(sizeof(sg_rtsp_t));
    rtsp->url = url;
    rtsp->sdp_filename = (char *) malloc(32);
    rtsp->control = (char *) malloc(32);
    rtsp->transport = url;
    rtsp->on_open = on_open;
    rtsp->on_data = on_data;
    rtsp->on_close = on_close;

    rtsp->curl = curl_easy_init();
    if (rtsp->curl != NULL) {
        FILE *sdp_fp;
        get_sdp_filename(rtsp->url, rtsp->sdp_filename, (size_t) sizeof(rtsp->url));
        sdp_fp = fopen(rtsp->sdp_filename, "wb");

        if (sdp_fp == NULL) {
            fprintf(stderr, "Could not open '%s' for writing\n", rtsp->sdp_filename);
            sdp_fp = stdout;
        } else {
            printf("Writing SDP to '%s'\n", rtsp->sdp_filename);
        }

        rtsp->uri = (char *) malloc(strlen(url) + 32);
        snprintf(rtsp->uri, strlen(rtsp->url) + 32, "%s", rtsp->url);
        curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_STREAM_URI, rtsp->uri);
        curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_REQUEST, (long) CURL_RTSPREQ_OPTIONS);
        //curl_easy_setopt(rtsp->curl,CURLOPT_WRITEFUNCTION,get_sdp);
        curl_easy_setopt(rtsp->curl, CURLOPT_WRITEDATA, sdp_fp);
        curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_REQUEST, (long) CURL_RTSPREQ_DESCRIBE);
        curl_easy_perform(rtsp->curl);

        curl_easy_setopt(rtsp->curl, CURLOPT_WRITEDATA, stdout);
        curl_easy_perform(rtsp->curl);
        printf("get_mdia_control_atrribute");
        get_media_control_attribute(rtsp);

        if (sdp_fp != stdout) {
            fclose(sdp_fp);
        }
    }

    return rtsp;
}

int sg_rtsp_run(sg_rtsp_t *r)
{

    CURLcode res = CURLE_OK;
    char *stream_data;
    rtsp->range = "0.00-";
    printf("\nRTSP: SETUP %s\n", rtsp->uri);
    printf("      TRANSPORT %s\n", rtsp->transport);
    snprintf(rtsp->uri, strlen(rtsp->url) + 32, "%s/%s", rtsp->url, rtsp->control);
    printf("%s\n", rtsp->uri);
    curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_STREAM_URI, rtsp->uri);
    curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_TRANSPORT, r->use_tcp ? rtsp_transport_tcp : rtsp_transport_udp);
    curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_REQUEST, (long) CURL_RTSPREQ_SETUP);
    curl_easy_perform(rtsp->curl);
    curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_STREAM_URI, rtsp->uri);
    curl_easy_setopt(rtsp->curl, CURLOPT_RANGE, rtsp->range);
    curl_easy_setopt(rtsp->curl, CURLOPT_RTSP_REQUEST, (long) CURL_RTSPREQ_PLAY);
    curl_easy_setopt(rtsp->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(rtsp->curl, CURLOPT_WRITEDATA, rtsp->on_data);
    res = curl_easy_perform(rtsp->curl);
    if (res == CURLE_OK) {
        printf("正在运行拉取实时视频!\n");
    } else {
        printf("运行失败!\n");
    }
    return (int) res;
}

void sg_rtsp_close(sg_rtsp_t *r)
{

    if (r->curl) {
        curl_easy_setopt(r->curl, CURLOPT_RTSP_REQUEST, (long) CURL_RTSPREQ_TEARDOWN);
        curl_easy_perform(r->curl);
        curl_easy_cleanup(r->curl);
    }
    if(r->url)
        free(r->url);
    free(r);
}

void sg_rtsp_cleanup(void)
{
    curl_global_cleanup();
}

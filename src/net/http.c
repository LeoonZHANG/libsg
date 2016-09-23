/**
 * http_client.c
 * HTTP client interfaces based on libcurl.
 */

/* http://blog.csdn.net/delphiwcdj/article/details/18284429 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <curl/curl.h>
#include <sg/sg.h>
#include <sg/net/http.h>
#include <sg/util/vsbuf.h>
#include <sg/str/string.h>

struct sg_http_real {
    CURL                        *curl;
    char                        *url;
    struct curl_slist           *header_list;
    char                        *post_fields;
    int                         http_version;       /* CURL_HTTP_VERSION_1_0 / CURL_HTTP_VERSION_1_1 / CURL_HTTP_VERSION_2_0 */
    char                        *request_method;
    unsigned int                conn_timeout_ms;
    unsigned int                recv_timeout_ms;
    sg_http_reply_func_t        cb;
    void                        *ctx;
    int                         async;
    struct sg_flex_buf          *resp_data;
    int                         verbose;            /* 0 / 1 */
};

size_t curl_write(void *buffer, size_t size, size_t count, void *user_ptr);

size_t curl_write(void *buffer, size_t size, size_t count, void *user_ptr)
{fprintf(stderr, "curl_write size %lu count %lu", size, count);
    size_t size_this_time = size * count;
    struct sg_flex_buf *flex_buf = (struct sg_flex_buf *)user_ptr;

    if (!flex_buf)
        return size_this_time;

    if (sg_flex_buf_insert(flex_buf, buffer, size_this_time) == 0) {
        sg_log_dbg("HTTP returns %lu bytes data.", size_this_time);
        return size_this_time;
    } else {
        sg_log_dbg("HTTP returns %lu bytes data, but save failure.", size_this_time);
        return 0;
    }
}

int sg_http_init(void)
{
    CURLcode res;

    res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        sg_log_err("curl_global_init failed : %s.", strerror(res));
        return -1;
    }

    return 0;
}

sg_http_t *sg_http_create(void)
{
    struct sg_http_real *client;

    client = (struct sg_http_real *)malloc(sizeof(struct sg_http_real));
    if (!client) {
        sg_log_err("malloc error.");
        return NULL;
    }

    memset(client, 0, sizeof(struct sg_http_real));
    client->http_version = CURL_HTTP_VERSION_1_1;

    client->resp_data = sg_flex_buf_create();
    if (!client->resp_data) {
        free(client);
        sg_log_err("Flex buf init error.");
        return NULL;
    }

    return client;
}

int sg_http_set_base(sg_http_t *client, const char *url, const char *request_method)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);
    SG_ASSERT(url);
    SG_ASSERT(strlen(url) > 0);
    SG_ASSERT(request_method);
    SG_ASSERT(strlen(request_method) > 0);

    if (c->url)
        free(c->url);

    c->url = strdup(url);
    c->request_method = strdup(request_method);
    sg_str_to_upper(c->request_method);

    return 0;
}

int sg_http_set_conn_timeout(sg_http_t *client, unsigned int ms)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);
    if (ms == 0)
        return -1;

    c->conn_timeout_ms = ms;
    return 0;
}

int sg_http_set_recv_timeout(sg_http_t *client, unsigned int ms)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);
    if (ms == 0)
        return -1;

    c->recv_timeout_ms = ms;
    return 0;
}

int sg_http_set_version(sg_http_t *client, const char *ver)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);
    SG_ASSERT(ver);

    if (strlen(ver) == 0)
        return -1;

#ifndef CURL_VERSION_HTTP2
    if (strcmp(ver, "2.0") == 0) {
        sg_log_err("Current libcurl version does not support HTTP 2.0.");
        return -1;
    }
#endif

    if (strcmp(ver, "1.0") == 0)
        c->http_version = CURL_HTTP_VERSION_1_0;
    else if (strcmp(ver, "1.1") == 0)
        c->http_version = CURL_HTTP_VERSION_1_1;
    else if (strcmp(ver, "2.0") == 0)
        c->http_version = CURL_HTTP_VERSION_2_0;
    else {
        sg_log_err("HTTP version %s is unsupported.", ver);
        return -1;
    }

    return 0;
}

int sg_http_set_async(sg_http_t *client)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);

    c->async = 1;

    return 0;
}

void sg_http_set_verbose(sg_http_t *client, int off_on)
{
    SG_ASSERT(client);

    if (off_on == 0)
        ((struct sg_http_real *)client)->verbose = 0;
    if (off_on == 1)
        ((struct sg_http_real *)client)->verbose = 1;
}

int sg_http_append_header(sg_http_t *client, const char *header)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);
    SG_ASSERT(header);

    if (strlen(header) == 0)
        return -1;

    c->header_list = curl_slist_append(c->header_list, header);
    return 0;
}

int sg_http_set_post_fields(sg_http_t *client, const char *post_fields)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);
    SG_ASSERT(post_fields);
    if (strlen(post_fields) == 0)
        return -1;

    c->post_fields = (char *)post_fields;

    return 0;
}

int sg_http_set_recv_callback(sg_http_t *client, sg_http_reply_func_t cb, void *ctx)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);
    SG_ASSERT(cb);

    c->cb = cb;
    c->ctx = ctx;

    return 0;
}

int sg_http_run(sg_http_t *client)
{
    struct sg_http_real *c = (struct sg_http_real *)client;
    struct sg_http_reply reply;
    long status_code;
    CURLoption request_method;
    CURLcode res;

    SG_ASSERT(client);

    if (!c->url || strlen(c->url) == 0 || !c->request_method || strlen(c->request_method) == 0) {
        sg_log_err("Error HTTP request parameters.");
        return -1;
    }

    c->curl = curl_easy_init();
    if (!c->curl) {
        sg_log_err("curl_easy_init returns NULL.");
        return -1;
    }

    if (strcmp(c->request_method, "GET") == 0)
        curl_easy_setopt(c->curl, CURLOPT_HTTPGET, 1);
    else if (strcmp(c->request_method, "POST") == 0)
        curl_easy_setopt(c->curl, CURLOPT_HTTPPOST, 1);
    else
        curl_easy_setopt(c->curl, CURLOPT_CUSTOMREQUEST, c->request_method);
    curl_easy_setopt(c->curl, CURLOPT_HTTP_VERSION, c->http_version);
    curl_easy_setopt(c->curl, CURLOPT_URL, c->url);
    curl_easy_setopt(c->curl, CURLOPT_VERBOSE, c->verbose);
    curl_easy_setopt(c->curl, CURLOPT_WRITEFUNCTION, &curl_write);
	curl_easy_setopt(c->curl, CURLOPT_HTTPGET, 1);
    if (c->header_list)
        curl_easy_setopt(c->curl, CURLOPT_HTTPHEADER, c->header_list);
    if (c->resp_data)
        curl_easy_setopt(c->curl, CURLOPT_WRITEDATA, c->resp_data);
    if (c->conn_timeout_ms > 0)
        curl_easy_setopt(c->curl, CURLOPT_CONNECTTIMEOUT_MS, c->conn_timeout_ms);
    if (c->recv_timeout_ms > 0)
        curl_easy_setopt(c->curl, CURLOPT_TIMEOUT_MS, c->recv_timeout_ms);
    if (c->post_fields) {
        curl_easy_setopt(c->curl, CURLOPT_POSTFIELDS, c->post_fields);
        curl_easy_setopt(c->curl, CURLOPT_POSTFIELDSIZE, strlen(c->post_fields) + 1);
    }fprintf(stderr, "resp_data %p", c->resp_data);
    if (c->resp_data)
        sg_flex_buf_reset(c->resp_data);
    res = curl_easy_perform(c->curl);
    curl_easy_getinfo(c->curl, CURLINFO_RESPONSE_CODE, &status_code);
    if (res != CURLE_OK)
        sg_log_err("curl_easy_perform error: %s, url: \"%s\", HTTP status code:%d.",
                   curl_easy_strerror(res), c->url, status_code);
    curl_easy_cleanup(c->curl);
    c->curl = NULL;

    if (c->cb) {
        memset(&reply, 0, sizeof(struct sg_http_reply));
        reply.status_code = status_code;
        reply.content = c->resp_data->mem;
        reply.content_size = c->resp_data->size;
        c->cb(&reply, c->ctx);
    }

    return (res == CURLE_OK) ? 0 : -1;
}

void sg_http_reset(sg_http_t *client)
{
    struct sg_http_real *c = (struct sg_http_real *)client;

    SG_ASSERT(client);

    if (c->url) {
        free(c->url);
        c->url = NULL;
    }
    if (c->request_method) {
        free(c->request_method);
        c->request_method = NULL;
    }
    if (c->header_list) {
        curl_slist_free_all(c->header_list);
        c->header_list = NULL;
    }
    if (c->resp_data)
        sg_flex_buf_reset(c->resp_data);
    c->post_fields = NULL;
    c->http_version = CURL_HTTP_VERSION_1_1;
    c->conn_timeout_ms = 0;
    c->recv_timeout_ms = 0;
    c->cb = NULL;
    c->ctx = NULL;
    c->async = 0;
    c->verbose = 0;
}

void sg_http_destroy(sg_http_t *client)
{
    SG_ASSERT(client);

    sg_http_reset(client);
    free(client);
}

void sg_http_quit(void)
{
    curl_global_cleanup();
}

int sg_http_easy_get(const char *url, sg_http_reply_func_t cb, void *ctx)
{
    sg_http_t *client;

    SG_ASSERT(url);

    client = sg_http_create();
    if (!client)
        return -1;

    sg_http_set_base(client, url, "GET");
    sg_http_set_recv_callback(client, cb, ctx);

    return sg_http_run(client);
}

int sg_http_easy_post(const char *url, const char *post_fields, sg_http_reply_func_t cb, void *ctx)
{
    sg_http_t *client;

    SG_ASSERT(url);

    client = sg_http_create();
    if (!client)
        return -1;

    sg_http_set_base(client, url, "POST");
    sg_http_set_post_fields(client, post_fields);
    sg_http_set_recv_callback(client, cb, ctx);

    return sg_http_run(client);
}

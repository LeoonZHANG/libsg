/**
 * http.h
 * HTTP client interfaces based on libcurl.
 */

#ifndef LIBSG_HTTP_H
#define LIBSG_HTTP_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* reply from HTTP server side */
struct sg_http_reply {
    int                     status_code;  /* HTTP status code */
    const void              *content;     /* POST data, or NULL */
    size_t                  content_size; /* data size */
};

typedef struct sg_http_real sg_http_t;

/* HTTP reply callback */
typedef void (*sg_http_reply_func_t)(struct sg_http_reply *reply, void *ctx);


/* Initialize http client library. */
int sg_http_init(void);

/* Create http client context. */
sg_http_t *sg_http_create(void);

/*
 * Set base parameters for context.
 * request_method: "GET" / "POST" / "YOUR_CUSTOM_REQUEST_METHOD"
 */
int sg_http_set_base(sg_http_t *client, const char *url, const char *request_method);

/* Set connection timeout for context. */
int sg_http_set_conn_timeout(sg_http_t *client, unsigned int ms);

/* Set receive timeout for context. */
int sg_http_set_recv_timeout(sg_http_t *client, unsigned int ms);

/*
 * Set http version for context, "1.1" is the default.
 * ver: "1.0" / "1.1" / "2.0"
 */
int sg_http_set_version(sg_http_t *client, const char *ver);

/* Set async run for context. */
int sg_http_set_async(sg_http_t *client);

/* Open or close libcurl verbose log. */
void sg_http_set_verbose(sg_http_t *client, int off_on);

/* Append header string for context. */
int sg_http_append_header(sg_http_t *client, const char *header);

/*
 * Set request string data for context, post_fields ends in \0.
 * Please don't free post_fields before sg_http_run();
 */
int sg_http_set_post_fields(sg_http_t *client, const char *post_fields);

/* Set return data callback function for context. */
int sg_http_set_recv_callback(sg_http_t *client, sg_http_reply_func_t cb, void *ctx);

/* Run it. */
int sg_http_run(sg_http_t *client);

/* Clear context content and make it to initial status. */
void sg_http_reset(sg_http_t *client);

/* Free context. */
void sg_http_destroy(sg_http_t *client);

/* Free http client library. */
void sg_http_quit(void);

/*
 * Easy http get interface, no need to call sg_http_init and sg_http_quit.
 * Returns error code.
 */
int sg_http_easy_get(const char *url, sg_http_reply_func_t cb, void *ctx);

/*
 * Easy http post interface, no need to call sg_http_init and sg_http_quit.
 * Returns error code.
 */
int sg_http_easy_post(const char *url, const char *post_fields, sg_http_reply_func_t cb, void *ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_HTTP_H */
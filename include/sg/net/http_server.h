/*
 * http_server.h
 * Author: wangwei.
 * Mini HTTP server package based on mongoose.
 */

#ifndef LIBSG_HTTP_SERVER_H
#define LIBSG_HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* HTTP request content from client */
struct sg_http_server_request {
    int                         status_code;     /* HTTP status code for HTTP error handler */
    const char                  *uri;            /* URL-decoded URI */
    const void                  *content;        /* POST (or websocket message) data, or NULL */
    size_t                      content_size;    /* data size */
    char                        *request_method; /* "GET" / "POST" / "CUSTOM_REQUEST_METHOD" */
    char                        *http_version;   /* "1.0" / "1.1" / "2.0" */
    const char                  *query_string;   /* URL part after '?', not including '?', or NULL */
    const char                  *client_ip;      /* client IP address */
    unsigned short              client_port;     /* client port */
    size_t                      header_size;     /* number of HTTP headers */
    struct http_headers {
        const char  *name;
        const char  *value;
    } headers[30];
};

/*
 * Please don't convert 'data' to char* and strcpy it as string,
 * the memory data after 'size' bytes are wrote last time, looks like random.
 * If this returns non-zero, http server will send http 200 to client;
 */
typedef void (*sg_http_server_request_func_t)(void *client_conn, struct sg_http_server_request *request, void *ctx);

/* Asynchronous or synchronous http server. */
enum sg_http_server_dispatch {
    SGHTTPSERVERDISPATCH_SYNC  = 0,
    SGHTTPSERVERDISPATCH_ASYNC = 1
};

/* HTTP server handle for user. */
typedef struct http_server_real sg_http_server;

/*
 * Create a http server, returns a handle.
 * Client connection will timeout if handler costs too much time, take care.
 */
sg_http_server *sg_http_server_create(const char *port, sg_http_server_request_func_t cb,
                                      enum sg_http_server_dispatch dispatch, void *ctx);

/*
 * Start the http server by handle.
 * It is asynchronous when http server is 'HTTPSERVERDISPATCH_ASYNC',
 * or synchronous when http server is 'HTTPSERVERDISPATCH_SYNC'.
 */
int sg_http_server_start(sg_http_server *s);

/* Reply HTTP header to client */
void sg_http_server_reply_header(void *client_conn, const char *name, const char *value);

/* Reply HTTP status code to client */
void sg_http_server_reply_status(void *client_conn, int status_code);

/* Reply data to client */
void sg_http_server_reply_data(void *client_conn, void *data, size_t size);

/* Stop the http server by handle. It is asynchronous. */
void sg_http_server_stop(sg_http_server *s);

/* Wait http server until it exit. */
void sg_http_server_join(sg_http_server *s);

/* Destroy the http server by handle, you should stop and join it first. */
void sg_http_server_destroy(sg_http_server **s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_HTTP_SERVER_H */
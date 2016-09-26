/**
 * webui_server.h
 * Mini HTTP server package based on mongoose for device dashboards.
 */

#ifndef LIBSG_WEBUI_SERVER_H
#define LIBSG_WEBUI_SERVER_H

#include <sg/sg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* HTTP request content from client */
struct sg_webui_server_request {
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
typedef void (*sg_webui_server_request_cb_t)(void *client_conn, struct sg_webui_server_request *request, void *ctx);

/* Asynchronous or synchronous webui server. */
enum sg_webui_server_dispatch {
    SGWEBUISERVERDISPATCH_SYNC  = 0,
    SGWEBUISERVERDISPATCH_ASYNC = 1
};

/* HTTP server handle for user. */
typedef struct webui_server_real sg_webui_server;

/*
 * Create a http server, returns a handle.
 * Client connection will timeout if handler costs too much time, take care.
 */
sg_webui_server *sg_webui_server_create(const char *port, sg_webui_server_request_cb_t cb,
                                      enum sg_webui_server_dispatch dispatch, void *ctx);

/*
 * Start the webui server by handle.
 * It is asynchronous when webui server is 'WEBUISERVERDISPATCH_ASYNC',
 * or synchronous when webui server is 'WEBUISERVERDISPATCH_SYNC'.
 */
int sg_webui_server_start(sg_webui_server *s);

/* Reply HTTP header to client */
void sg_webui_server_reply_header(void *client_conn, const char *name, const char *value);

/* Reply HTTP status code to client */
void sg_webui_server_reply_status(void *client_conn, int status_code);

/* Reply data to client */
void sg_webui_server_reply_data(void *client_conn, void *data, size_t size);

/* Stop the webui server by handle. It is asynchronous. */
void sg_webui_server_stop(sg_webui_server *s);

/* Wait webui server until it exit. */
void sg_webui_server_join(sg_webui_server *s);

/* Destroy the webui server by handle, you should stop and join it first. */
void sg_webui_server_destroy(sg_webui_server **s);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_WEBUI_SERVER_H */
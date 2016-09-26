/**
 * webui_server.c
 * Mini HTTP server package based on mongoose for device dashboards.
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sg/sg.h>
#include <sg/sys/flag.h>
#include <sg/sys/thread.h>
#include <sg/str/string.h>
#include <sg/net/webui_server.h>
#include "../../3rdparty/mongoose_5.6/mongoose.h"

/* HTTP server context */
struct webui_server_real {
    sg_flag_t                       *run_flag;
    struct mg_server                *server;
    sg_webui_server_request_cb_t    client_request_cb;
    void                            *ctx;
    sg_thread_t                     thread;
    enum sg_webui_server_dispatch   dispatch;
};

void webui_server_async_run(sg_webui_server *s);
void webui_server_sync_run(void *s);

/* event handler */
static int ev_handler(struct mg_connection *conn, enum mg_event ev)
{
    size_t i;
    struct webui_server_real *s;
    struct sg_webui_server_request request;

    switch (ev) {
    case MG_AUTH:
        return MG_TRUE; /* authenticated */
    case MG_REQUEST:
        /* What happened after resending status? Send it right now or latter? */
        /* mg_send_header(conn, "Content-Type", "text/plain"); */
        /* mg_send_status(conn, 200); */
        /* mg_printf_data(conn, "This is a reply from server instance # %s",
                       (char *) conn->server_param); */
        sg_log_inf("HTTP server receives a %s request.", conn->request_method);
        s = (struct webui_server_real *)conn->server_param;
        if (s && s->client_request_cb) {
            memset(&request, 0, sizeof(struct sg_webui_server_request));
            request.uri = conn->uri;
            request.client_ip = conn->remote_ip;
            request.client_port = conn->remote_port;
            request.http_version = (char *)conn->http_version;
            request.request_method = (char *)conn->request_method;
            if (conn->content_len) {
                request.content = conn->content;
                request.content_size = conn->content_len;
            }
            for (i = 0; i < conn->num_headers; i++) {
                request.headers[i].name = conn->http_headers[i].name;
                request.headers[i].value = conn->http_headers[i].value;
            }
            request.header_size = conn->num_headers;
            request.query_string = conn->query_string;
            s->client_request_cb(conn, &request, s->ctx);
        }
        return MG_TRUE;
    case MG_CLOSE:
        /* fprintf(stdout, "%s:%u went away\n", conn->remote_ip, conn->remote_port); */
        free(conn->connection_param);
        conn->connection_param = NULL;
        return MG_TRUE; /* Callback return value is ignored. */
    default:
        return MG_FALSE; /* Keep the connection open. */
    }
}

void sg_webui_server_reply_header(void *client_conn, const char *name, const char *value)
{
    SG_ASSERT(client_conn);
    SG_ASSERT(name);
    SG_ASSERT(value);

    if (strlen(name) == 0 || strlen(value) == 0)
        return;

    mg_send_header((struct mg_connection *)client_conn, name, value);
}

void sg_webui_server_reply_status(void *client_conn, int status_code)
{
    SG_ASSERT(client_conn);
    SG_ASSERT(status_code >= 0);

    mg_send_status((struct mg_connection *)client_conn, status_code);
}

void sg_webui_server_reply_data(void *client_conn, void *data, size_t size)
{
    SG_ASSERT(client_conn);
    SG_ASSERT(data);
    SG_ASSERT(size > 0);

    mg_send_data((struct mg_connection *)client_conn, data, size);
}

sg_webui_server *sg_webui_server_create(const char *port, sg_webui_server_request_cb_t cb,
                                      enum sg_webui_server_dispatch dispatch, void *ctx)
{
    struct webui_server_real *s;

    SG_ASSERT(port);
    SG_ASSERT(strlen(port) > 0);

    if (sg_str_is_decimal(port) != 1) {
        sg_log_err("Port %s is not decimal.", port);
        return NULL;
    }

    /* Create and configure HTTP server. */
    s = (struct webui_server_real *)malloc(sizeof(struct webui_server_real));
    if (!s)
        return NULL;
    sg_log_inf("HTTP server creating.");
    s->run_flag = sg_flag_create();
    s->client_request_cb = cb;
    s->dispatch = dispatch;
    s->ctx = ctx;
    s->server = mg_create_server((void *)s, ev_handler);
    mg_set_option(s->server, "listening_port", port);
    return s;
}

void webui_server_sync_run(void *p)
{
    struct webui_server_real *s = (struct webui_server_real *)p;

    SG_ASSERT(s);

    /* Run HTTP server. */
    sg_log_inf("HTTP server starting on port %s.", mg_get_option(s->server, "listening_port"));

    while (sg_flag_read(s->run_flag) == 1)
        mg_poll_server(s->server, 1000);

    /* Cleanup, and free server instance. */
    sg_log_inf("HTTP server exits.");
    mg_destroy_server(&(s->server));
    s->server = NULL;
}

void webui_server_async_run(sg_webui_server *s)
{
    SG_ASSERT(s);

    sg_thread_init(&(s->thread), webui_server_sync_run, (void*)s);
}

int sg_webui_server_start(sg_webui_server *s)
{
    SG_ASSERT(s);

    sg_flag_write(s->run_flag, 1);

    if (s->dispatch == SGHTTPSERVERDISPATCH_ASYNC)
        webui_server_async_run(s);
    else
        webui_server_sync_run(s);

    return 0;
}

void sg_webui_server_stop(sg_webui_server *s)
{
    SG_ASSERT(s);

    sg_flag_write(s->run_flag, 0);
}

void sg_webui_server_join(sg_webui_server *s)
{
    SG_ASSERT(s);

    sg_thread_join(&(s->thread));
}

void sg_webui_server_destroy(sg_webui_server **s)
{
    SG_ASSERT(s);
    if (!(*s))
        return;

    if (sg_flag_read((*s)->run_flag) == 1) {
        sg_log_err("HTTP server is running, cann\'t be destroyed.");
        return;
    }
    sg_flag_destroy(&((*s)->run_flag));
    free(*s);
    *s = NULL;
}
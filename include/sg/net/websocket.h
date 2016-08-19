/*
 * websocket.h
 * Easy to use websocket client library (rfc 6455) based on mongoose.
 */

#ifndef LIBSG_WEBSOCKET_H
#define LIBSG_WEBSOCKET_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct sg_websocket_in sg_websocket_t;

typedef void (*sg_websocket_on_open_func_t)(sg_websocket_t *);
typedef void (*sg_websocket_on_message_func_t)(sg_websocket_t *, const char *data, size_t size);
typedef void (*sg_websocket_on_close_func_t)(sg_websocket_t *, int code, const char *reason);

/*
 * url: such like "ws://12.11.230.12:3000".
 * sync/async: async.
 */
sg_websocket_t *
sg_websocket_open(const char *url,
				  sg_websocket_on_open_func_t on_open,
				  sg_websocket_on_message_func_t on_message,
				  sg_websocket_on_close_func_t on_close);

/*
 * function: run this connection until it is closed.
 * sync/async: sync.
 * interval_ms: poll interval time in msec.
 */
int
sg_wwebsocket_loop(sg_websocket_t *, int interval_ms);

/* sync/async: sync. */
int
sg_wwebsocket_send(sg_websocket_t *, const char *data, size_t);

/* sync/async: sync. */
void
sg_wwebsocket_close(sg_websocket_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBSG_WEBSOCKET_H */
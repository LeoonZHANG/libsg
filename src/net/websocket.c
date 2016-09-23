/*
 * websocket.c
 * Easy to use websocket client library (rfc 6455) based on mongoose.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sg/sg.h>

#include "../../include/net/websocket.h"
#include "../../3rdparty/mongoose_6.2/mongoose.h"

enum sg_websocket_state {
	SGWEBSOCKETSTATE_CONNECTING,
	SGWEBSOCKETSTATE_OPEN,
	SGWEBSOCKETSTATE_CLOSED
};

struct sg_websocket_in {
	struct mg_mgr 			mgr;
	struct mg_connection 	*conn;
	char 					*url;
	enum sg_websocket_state 		state;
	sg_websocket_on_open_func_t		on_open;
	sg_websocket_on_message_func_t	on_message;
	sg_websocket_on_close_func_t	on_close;
};

static void websocket_free(sg_websocket_t *ws)
{
	struct sg_websocket_in *w = (struct sg_websocket_in *)ws;
	
	if (w) {
		if (w->conn) {
			mg_close_conn(w->conn);
			w->conn = NULL;
		}
		mg_mgr_free(&w->mgr);
		free(w->url);
		free(w);
	}
}

static inline int is_op(int flags, int op)
{
	return (flags & op) == op;
}

/* handle common frame data */
static void handle_frame(sg_websocket_t *ws, struct websocket_message *wm)
{
	struct sg_websocket_in *w = (struct sg_websocket_in *)ws;

	/* wm->flags: WEBSOCKET_OP_TEXT or WEBSOCKET_OP_BINARY, you can ignore this */
	if (w->state == SGWEBSOCKETSTATE_OPEN)
		if (w->on_message)
			w->on_message(ws, wm->data, wm->size);
}

/* handle keepalive data */
static void handle_control_frame(sg_websocket_t *ws, struct websocket_message *wm)
{
	struct sg_websocket_in *w = (struct sg_websocket_in *)ws;
	int code = 0;
	char *reason = NULL;
	struct mg_connection *nc = w->conn;

	printf("====>on control frame = %d\n", wm->flags);

	if (is_op(wm->flags, WEBSOCKET_OP_PING)) { /* return data same as received */
		printf("====>on ping = %.*s\n", wm->size, wm->data);
		mg_send_websocket_frame(nc, WEBSOCKET_OP_PONG, wm->data, wm->size);
	} else if (is_op(wm->flags, WEBSOCKET_OP_PONG)) /* for keeping connection alive */
		printf("====>on pong = %.*s\n", wm->size, wm->data);
	else if (is_op(wm->flags, WEBSOCKET_OP_CLOSE)) { /* Websocket server closed this connection */
		printf("====>on websocket close\n");
		if (w->state != SGWEBSOCKETSTATE_CLOSED) {
			w->state = SGWEBSOCKETSTATE_CLOSED;
			if (w->on_close) {
				/* Head: 2-byte unsigned integer in network byte order (big-endian).
				   Body: string in utf-8. */
				if (wm->size >= 2) {
					code = (int) *wm->data << 8;
					code += (int) *(wm->data + 1);
					if (wm->size > 2) {
						int reason_size = wm->size - 2;
						reason = malloc(reason_size + 1);
						strncpy(reason, wm->data + 2, reason_size);
						reason[reason_size] = '\0';
					}
				}
				w->on_close(ws, code, reason);
				free(reason);
			}
		}
		nc->flags |= MG_F_CLOSE_IMMEDIATELY;
	}
}

/* handle open event */
static void handle_open(sg_websocket_t *ws)
{
	struct sg_websocket_in *w = (struct sg_websocket_in *)ws;

	w->state = SGWEBSOCKETSTATE_OPEN;
	if (w->on_open)
		w->on_open(ws);
}

/* handle close event */
static void handle_close(sg_websocket_t *ws)
{
	struct sg_websocket_in *w = (struct sg_websocket_in *)ws;

	printf("===>onclose\n");
	if (w->state != SGWEBSOCKETSTATE_CLOSED) {
		w->state = SGWEBSOCKETSTATE_CLOSED;
		if (w->on_close)
			w->on_close(ws, 1001, NULL);
	}
}

/* event handler */
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	struct sg_websocket_in *ws = nc->user_data;
	struct websocket_message *wm = (struct websocket_message *)ev_data;
	int *success = (int *)ev_data;

	switch (ev) {
	case MG_EV_CONNECT:
		printf("====>connect=%d\n", *success);
		break;
	case MG_EV_POLL:
		printf("===>onpoll\n");
		break;
	case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
		handle_open(ws);
		break;
	case MG_EV_CLOSE:
		handle_close(ws);
		break;
	case MG_EV_WEBSOCKET_FRAME:
		handle_frame(ws, wm);
		break;
	case MG_EV_WEBSOCKET_CONTROL_FRAME:
		handle_control_frame(ws, wm);
		break;
	default:
		break;
	}
}

sg_websocket_t *
sg_websocket_open(const char *url,
				  sg_websocket_on_open_func_t on_open,
				  sg_websocket_on_message_func_t on_message,
				  sg_websocket_on_close_func_t on_close)
{
	struct sg_websocket_in *ws;

	ws = (struct sg_websocket_in *)malloc(sizeof(struct sg_websocket_in));
	if (!ws) {
		fprintf(stderr, "malloc error.\n");
		return NULL;
	}
	memset(ws, 0, sizeof(struct sg_websocket_in));
	ws->state 		= SGWEBSOCKETSTATE_CONNECTING;
	ws->on_open 	= on_open;
	ws->on_close 	= on_close;
	ws->on_message 	= on_message;

	ws->url = strdup(url);
	mg_mgr_init(&ws->mgr, NULL);

	ws->conn = mg_connect_ws(&ws->mgr, ev_handler, url, NULL, NULL);
	if (!ws->conn) {
		websocket_free(ws);
		return NULL;
	}
	ws->conn->user_data = ws;

	return ws;
}

int sg_websocket_loop(sg_websocket_t *ws, int interval_ms)
{
	while (ws->state != SGWEBSOCKETSTATE_CLOSED)
		mg_mgr_poll(&ws->mgr, interval_ms);

	websocket_free(ws);
	return 0;
}

int sg_websocket_send(sg_websocket_t *ws, const char *data, size_t size)
{
	if (ws->state == SGWEBSOCKETSTATE_OPEN) {
		mg_send_websocket_frame(ws->conn, WEBSOCKET_OP_BINARY, data, size);
		return 0;
	} else
		return -1;
}

void sg_websocket_close(sg_websocket_t *ws)
{
	if (ws->state == SGWEBSOCKETSTATE_CLOSED)
		return;

	ws->state = SGWEBSOCKETSTATE_CLOSED;
	if (ws->on_close)
		ws->on_close(ws, 1000, NULL);
	ws->conn->flags |= MG_F_CLOSE_IMMEDIATELY;
	mg_close_conn(ws->conn);
}
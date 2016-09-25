/*
 * rudp.h
 * Easy to use udp client library based on enet.
 */

#ifndef RUDP_H
#define RUDP_H

#include <stdint.h>
#include <enet/enet.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct rudp_in;

typedef struct rudp_in rudp_t;

typedef void (*rudp_on_open)(rudp_t *client);

typedef void (*rudp_on_message)(rudp_t *client,char *data, size_t size);

typedef void (*rudp_on_close)(rudp_t *client);

//typedef void (*rudp_on_error)(rudp_t *client,const char *err_msg);

enum RUDP_STATE {
	RUDP_STATE_CONNECTING, RUDP_STATE_OPEN,
	RUDP_STATE_CLOSTING,
	RUDP_STATE_CLOSED
};
struct rudp_in {
	ENetHost *host;
	ENetPeer *peer;
	enum RUDP_STATE state;
	void *user_data;

	rudp_on_open on_open;
	rudp_on_message on_message;
	//rudp_on_error on_error;
	rudp_on_close on_close;
};

/**
 * 初始化，返回0表示成功
 */
int rudp_init();
/**
 *
 */
rudp_t *rudp_open(const char *server_addr,
		int server_port,
                            rudp_on_open on_open,
                            rudp_on_message on_message,
                            rudp_on_close on_close
                           /*,rudp_on_error on_error,void *user_data*/);

int rudp_loop(rudp_t *client, int interval_ms);

int rudp_send(rudp_t *client, const void *data, uint64_t size);

void rudp_close(rudp_t *client);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RUDP_H */

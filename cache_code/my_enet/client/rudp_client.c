/*
 ============================================================================
 Name        : udp_client.c
 Author      : cole
 Version     :
 Copyright   : cole
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <enet/enet.h>

#include "rudp_client.h"

//可以注释掉或者在编译的时候定义
#define RUDB_DEBUG

#ifdef RUDP_DEBUG
#define log_debug(...)  printf(__VA_ARGS__)
#else
#define log_debug
#endif

#define log_error(...) printf(__VA_ARGS__)

static void rudp_free(rudp_t *client) {
	if (client != NULL) {
		//enet_peer_disconnect(client->peer,0);
		enet_host_destroy(client->host);
		client->host = NULL;
		free(client);
	}

}
int rudp_init() {
	if (enet_initialize() != 0) {
		log_error("An error occurred while initializing ENet.\n");
		return -1;
	}
	atexit(enet_deinitialize);
	return 0;
}
rudp_t *rudp_open(const char *server_addr, int server_port,
		rudp_on_open on_open, rudp_on_message on_message,
		rudp_on_close on_close/*,rudp_on_error on_error,void *user_data*/) {

	ENetHost * host;
	host = enet_host_create(NULL, 1, 1, 0, 0);
	if (host == NULL) {
		log_error(
				"An error occurred while trying to create an ENet client host.\n");
		return NULL;
	}

	ENetAddress address;
	ENetPeer *peer;
	enet_address_set_host(&address, server_addr);
	address.port = server_port;
	peer = enet_host_connect(host, &address, 1, 0);
	if (peer == NULL) {
		log_error("No available peers for initiating an ENet connection.\n");
		enet_host_destroy(host);
		return NULL;
	}
	rudp_t *client = malloc(sizeof(rudp_t));
	if (client == NULL) {
		log_error("malloc error\n");
		enet_peer_disconnect_now(peer, 0);
		enet_host_destroy(host);
		return NULL;
	}
	memset(client, 0, sizeof(rudp_t));
	client->state = RUDP_STATE_CONNECTING;
	client->peer = peer;
	client->host = host;

	client->on_open = on_open;
	client->on_message = on_message;
	client->on_close = on_close;
	//client->on_error = on_error;

	return client;

}
void handle_connect(rudp_t* client, ENetEvent *event) {
	log_debug("on connected\n");
	client->state = RUDP_STATE_OPEN;
	if (client->on_open) {
		client->on_open(client);
	}
}
void handle_disconnect(rudp_t* client, ENetEvent *event) {
	log_debug("on disconnected.\n");
	if (client->state != RUDP_STATE_CLOSED) {
		client->state = RUDP_STATE_CLOSED;
		if (client->on_close) {
			client->on_close(client);
		}
	}

}
void handle_receive(rudp_t* client, ENetEvent *event) {
	log_debug("on message\n");
	if (client->state == RUDP_STATE_OPEN) {
		if (client->on_message) {
			client->on_message(client, event->packet->data,
					event->packet->dataLength);
		}

	}
	/* Clean up the packet now that we're done using it. */
	enet_packet_destroy(event->packet);
}
int rudp_loop(rudp_t *client, int interval_ms) {
	ENetEvent event;
	while (client->state != RUDP_STATE_CLOSED
			&& enet_host_service(client->host, &event, interval_ms) >= 0) {
		switch (event.type) {
		//对于client，就是连接server成功
		case ENET_EVENT_TYPE_CONNECT:
			handle_connect(client, &event);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			handle_disconnect(client, &event);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			handle_receive(client, &event);
			break;
		case ENET_EVENT_TYPE_NONE:
			log_debug("on poll\n");
			break;
		}
	}

	rudp_free(client);
	return 0;
}

//返回0表示成功
int rudp_send(rudp_t *client, const void *data, uint64_t size) {
	if (client->state != RUDP_STATE_OPEN) {
		return -1;
	}
	ENetPacket * packet = enet_packet_create(data, size,
			ENET_PACKET_FLAG_RELIABLE);
	//发送后会自动释放packet
	//现在只设置一个channel，id从0开始
	return enet_peer_send(client->peer, 0, packet);
}

void rudp_close(rudp_t *client) {
	if (client->state == RUDP_STATE_CLOSED
			|| client->state == RUDP_STATE_CLOSTING) {
		return;
	}
	client->state = RUDP_STATE_CLOSTING;
	//如果关闭成功，会发出disconnect事件
	enet_peer_disconnect(client->peer, 0);

}

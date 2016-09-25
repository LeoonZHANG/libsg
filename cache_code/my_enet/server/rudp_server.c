/*
 ============================================================================
 Name        : udp_server.c
 Author      : cole
 Version     :
 Copyright   : cole
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include <enet/enet.h>

#include "rudp_server.h"

//#define RUDP_SERVER_DEBUG

//typedef void (*rudp_server_on_client_open_func_t)(rudp_client_t *conn);
//typedef void (*rudp_server_on_client_close_func_t)(rudp_client_t *conn);
//typedef void (*rudp_server_on_client_data_func_t)(rudp_client_t *conn, void *data, size_t len);
//typedef void (*rudp_server_on_client_error_func_t)(const char *errmsg, rudp_client_t *conn);
//typedef void (*rudp_server_on_iter_func_t)(rup_server_t *, rudp_server_conn_t *, void *user_data);

#ifdef RUDP_SERVER_DEBUG
#define log_debug(...)  printf(__VA_ARGS__)
#else
#define log_debug
#endif

#define log_error(...) printf(__VA_ARGS__)

int rudp_init() {
	if (enet_initialize() != 0) {
		log_debug("An error occurred while initializing ENet.\n");
		return -1;
	}
	atexit(enet_deinitialize);
	return 0;
}

static void rudp_server_free(rudp_server_t *server) {
	if (server) {
		enet_host_destroy(server->host);
		server->host = NULL;
		free(server);
	}
}

//
rudp_server_t *rudp_server_open(const char *server_addr, int server_port,
		int max_conn_size, rudp_server_on_client_open_func_t on_client_open,
		rudp_server_on_client_message_func_t on_client_message,
		rudp_server_on_client_close_func_t on_client_close
		//,rudp_server_on_client_error_func_t on_client_error
		) {

	ENetAddress address;
	ENetHost * host;

	address.host = ENET_HOST_ANY;
	if (server_addr) {
		enet_address_set_host(&address, server_addr);
	}
	address.port = server_port;
	host =
			enet_host_create(
					&address /* the address to bind the server host to */,
					max_conn_size /* allow up to 32 clients and/or outgoing connections */,
					1 /* allow up to 2 channels to be used, 0 and 1 */,
					0 /* assume any amount of incoming bandwidth */,
					0 /* assume any amount of outgoing bandwidth */);

	if (!host) {
		log_error("创建host失败\n");
		return NULL;
	}

	rudp_server_t *server = malloc(sizeof(rudp_server_t));
	if (!server) {
		log_error("malloc error\n");
		return NULL;
	}
	server->host = host;
	server->on_client_open = on_client_open;
	server->on_client_message = on_client_message;
	server->on_client_close = on_client_close;
	//server->on_client_error = on_client_error;

	return server;
}

int rudp_server_send_data(rudp_client_t *client, void *data, size_t size) {
	ENetPacket *packet = enet_packet_create(data, size,
			ENET_PACKET_FLAG_RELIABLE);
	//发送后会自动释放packet
	//只有一个channel，从0开始
	return enet_peer_send(client->peer, 0, packet);
}

void rudp_server_close_client(rudp_client_t *client) {
	enet_peer_disconnect(client->peer, 0);
}

char* rudp_server_get_client_addr(rudp_client_t * client) {
	//现在仅仅支持ip4=255.255.255.255
	char buf[16];
	memset(buf, 0, strlen(buf));
	enet_uint32 host = client->peer->address.host;
	host = ntohl(host);

	sprintf(buf, "%d.%d.%d.%d", (enet_uint8) (host >> 24),
			(enet_uint8) (host >> 16), (enet_uint8) (host >> 8),
			(enet_uint8) host);


	//char *copy = strdup(buf);
	//printf("ip=%s,buf=%s\n",copy,buf);
	return strdup(buf);

}

/* returns count of all clients */
int rudp_server_iter_client(rudp_server_t *server,
		rudp_server_on_iter_func_t on_iter_client, void *user_data) {
	int size = server->host->peerCount;
	int count = 0;
	for (int i = 0; i < size; i++) {
		ENetPeer *peer = &server->host->peers[i];
		//还没有连接成功或者断开连接了没有被从数组从删除?
		if (peer->data) {
			count++;
			rudp_client_t *client = peer->data;
			on_iter_client(server, client, user_data);
		} else {
			printf("====可能执行到这里吗？\n");
		}
	}
	return count;

}

static void handle_connect(rudp_server_t *server, ENetEvent *event) {
	log_debug("connect\n");
	rudp_client_t *client = malloc(sizeof(rudp_client_t));
	memset(client, 0, sizeof(rudp_client_t));
	client->peer = event->peer;
	event->peer->data = client;

	if (server->on_client_open) {
		server->on_client_open(client);
	}
}
static void handle_disconnect(rudp_server_t *server, ENetEvent *event) {
	log_debug("disconnected.\n");
	rudp_client_t *client = (rudp_client_t *) event->peer->data;
	event->peer->data = NULL;

	if (server->on_client_close && client) {
		server->on_client_close(client);
	}
	//释放client
	free(client);
}
static void handle_receive(rudp_server_t *server, ENetEvent *event) {
	ENetPacket *packet = event->packet;
	/* Clean up the packet now that we're done using it. */
	if (server->on_client_message) {
		rudp_client_t *client = (rudp_client_t*) event->peer->data;
		if (client) {
			server->on_client_message(client, packet->data, packet->dataLength);
		}

	}
	//必须销毁
	enet_packet_destroy(packet);
}
void rudp_server_run(rudp_server_t *server, int interval_ms) {
	//char *ip = rudp_server_get_client_addr()
	log_debug("listen on %x,port=%d\n", server->host->address.host, server->host->address.port);
	ENetEvent event;
	/* Wait up to 1000 milliseconds for an event. */
	while (enet_host_service(server->host, &event, interval_ms) >= 0) {
		//log_debug("event=%d\n", event.type);
		switch (event.type) {
		//对于client，就是连接server成功
		case ENET_EVENT_TYPE_CONNECT:
			handle_connect(server, &event);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			handle_disconnect(server, &event);
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			handle_receive(server, &event);
			break;

		case ENET_EVENT_TYPE_NONE:
			//printf("on poll\n");
			break;
		}
	}

	rudp_server_free(server);
}

void rudp_server_close(rudp_server_t * server) {
	enet_host_destroy(server->host);
	server->host = NULL;
	//free(server);
}


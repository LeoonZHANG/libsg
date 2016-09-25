/* 
 * rudp_server.h
 * reliable udp server library based on enet
 * 内部管理多个客户端连接时，请使用linkhash模块
 */

#ifndef RUDP_SERVER_H
#define	RUDP_SERVER_H

#include <stdlib.h>
#include <enet/enet.h>


#ifdef	__cplusplus
extern "C"
{
#endif


struct rudp_client;
struct rudp_server_real;

typedef struct rudp_client rudp_client_t;
typedef struct rudp_server_real rudp_server_t;

typedef void (*rudp_server_on_client_open_func_t)(rudp_client_t *client);
typedef void (*rudp_server_on_client_close_func_t)(rudp_client_t *client);
typedef void (*rudp_server_on_client_message_func_t)(rudp_client_t *client, char *data, size_t len);
//typedef void (*rudp_server_on_client_error_func_t)(rudp_client_t *conn,const char *errmsg);
typedef void (*rudp_server_on_iter_func_t)(rudp_server_t *server, rudp_client_t *client, void *user_data);



struct rudp_client {
	ENetPeer *peer;

	//可以设置任意的数据
	char *user_data;
};

struct rudp_server_real {
	ENetHost *host;
	void *user_data;

	rudp_server_on_client_open_func_t on_client_open;
	rudp_server_on_client_message_func_t on_client_message;
	rudp_server_on_client_close_func_t on_client_close;
//rudp_server_on_client_error_func_t on_client_error;
};


int rudp_init();

rudp_server_t *rudp_server_open(const char *server_addr,
                                int server_port,
                                int max_conn_size,
                                rudp_server_on_client_open_func_t,
								rudp_server_on_client_message_func_t,
                                rudp_server_on_client_close_func_t
                               );

//void rudp_server_set_max_conn(rudp_server_t *, int);

int rudp_server_send_data(rudp_client_t *, void *data, size_t size);

void rudp_server_close_client(rudp_client_t *);

//调用者需要释放返回的char*
char* rudp_server_get_client_addr(rudp_client_t *);

/* returns count of all clients */
int rudp_server_iter_client(rudp_server_t *, rudp_server_on_iter_func_t, void *user_data);

void rudp_server_run(rudp_server_t *,int interval_ms);

void rudp_server_close(rudp_server_t *);


#ifdef	__cplusplus
}
#endif

#endif	/* RUDP_SERVER_H */


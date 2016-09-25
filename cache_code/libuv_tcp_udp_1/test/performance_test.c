/* 
 * File:   performance_test.c
 * Author: sundq
 *
 * Created on 2016年1月31日, 下午1:56
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../tcp.h"
#include "../udp.h"

char *send_data = NULL;
tcp_client_t **tcp_clients = NULL;
size_t  byte_size = 1024;
uv_mutex_t mutex_handle = {0};
static void on_open(tcp_client_t *tcp_client)
{
    tcp_client_send_data(tcp_client, send_data, byte_size);
}


static void on_close(tcp_client_t *conn)
{
	fprintf(stderr, "connect close.\n");
}

static void on_error(const char *msg, tcp_client_t *conn)
{
	perror("something failed");
}
 
static void on_data(tcp_client_t *tcp_client, void *data, size_t len)
{
    printf("get data %zd\n", len);
	sleep(1);
	tcp_client_send_data(tcp_client, data, len);
}

static void on_send(tcp_client_t *conn, int status, void *data, size_t len)
{
    char *p = (char *) data;
    if (status == 0)
    {
//        printf("data send ok:%zd\n", len);
    }
    else
    {
//        printf("data send failed\n");
    }
}


/*
 * 

 */
int main(int argc, char** argv)
{
	int concurrence = 10;
	int time_go = 3; //60s
	byte_size = 1024;
	if (argc > 1)
	{
		concurrence = atoi(argv[1]);	
	}
	if (argc > 2)
	{
		time_go = atoi(argv[2]);
	}
	
	if (argc > 3)
	{
		byte_size = atoi(argv[3]);
	}
	
	send_data = malloc(byte_size);
	printf("current args: concurrence=%d time=%d byte_size=%zd.\n", concurrence, time_go, byte_size);
	sleep(1);
	tcp_clients = (tcp_client_t **)malloc(4 * concurrence);
	int i = 0;
	for (i=0; i<concurrence; i++)
	{
		printf("当前连接数：%d\n", i);
		tcp_client_t *tcp_client = tcp_client_create("127.0.0.1", 7000, on_open, on_data, on_close, on_error, on_send);
		tcp_clients[i] = tcp_client;
		usleep(100);
	}
    tcp_client_run();
}

